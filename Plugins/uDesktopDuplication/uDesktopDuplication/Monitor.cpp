#include <d3d11.h>
#include <ShellScalingAPI.h>
#include <queue>
#include "Debug.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Monitor.h"

#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;


///
/// Thraed safe self created ID3D11Device from specified adapter
///
class IsolatedD3D11Device
{
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11Texture2D> m_copyTarget;

public:
	ComPtr<ID3D11Device> GetDevice()const { return m_device; }

	bool CreateDevice(const ComPtr<IDXGIAdapter> &adapter)
	{
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT // D2D Compatible
													  //| D3D11_CREATE_DEVICE_SINGLETHREADED
													  //| D3D11_CREATE_DEVICE_VIDEO_SUPPORT // MediaFoundation
			;

		D3D_FEATURE_LEVEL FeatureLevelsRequested[6] = {
			D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };

		UINT               numLevelsRequested = sizeof(FeatureLevelsRequested) / sizeof(D3D_FEATURE_LEVEL);
		D3D_FEATURE_LEVEL  FeatureLevelsSupported;

		if (FAILED(D3D11CreateDevice(adapter.Get()
			, adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, nullptr
			, flags
			, FeatureLevelsRequested, numLevelsRequested
			, D3D11_SDK_VERSION
			, &m_device
			, &FeatureLevelsSupported
			, nullptr
		))) {
			return false;
		}
		return true;
	}

	ComPtr<ID3D11Texture2D> CreateCopyTargetWithSharedFlag(const ComPtr<ID3D11Texture2D> &src)
	{
		D3D11_TEXTURE2D_DESC desc;
		src->GetDesc(&desc);

		if (m_copyTarget) {
			D3D11_TEXTURE2D_DESC target_desc;
			m_copyTarget->GetDesc(&target_desc);
			if (
				target_desc.Format == desc.Format
				&& target_desc.Width == desc.Width
				&& target_desc.Height == desc.Height
				) {
				// already created
				return m_copyTarget;
			}
		}

		//desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE; 
		desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // for share texture with unity device

		if (FAILED(m_device->CreateTexture2D(&desc, NULL, &m_copyTarget)))
		{
			return nullptr;
		}
		return m_copyTarget;
	}
};


struct QueueItem
{
	ComPtr<ID3D11Texture2D> Texture;
	DXGI_OUTDUPL_FRAME_INFO Info;

	QueueItem()
	{}

	QueueItem(const ComPtr<ID3D11Texture2D> &texture
		, const DXGI_OUTDUPL_FRAME_INFO &info
	)
		: Texture(texture), Info(info)
	{}
};
class TextureQueue
{
	std::queue<QueueItem> m_queue;
	std::mutex m_mutex;

public:
	void Enqueue(const QueueItem &item)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		m_queue.push(item);
	}

	QueueItem Dequeue()
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		if (m_queue.empty()) {
			return QueueItem();
		}
		auto front = m_queue.front();
		m_queue.pop();
		return front;
	}
};


Monitor::Monitor(int id)
    : id_(id)
{
	ZeroMemory(&monitorInfo_, sizeof(monitorInfo_));
}


Monitor::~Monitor()
{
	m_stopLoop = true;

    if (deskDupl_) 
    {
        deskDupl_->Release();
        deskDupl_ = nullptr;
    }

	if (m_desktopDuplicationThread.joinable()) {
		m_desktopDuplicationThread.join();
	}
}


void Monitor::Initialize(
	const ComPtr<IDXGIAdapter> &adapter,
    const ComPtr<IDXGIOutput> &output
)
{
	m_pIsolated = std::make_shared<IsolatedD3D11Device>();
	m_textureQueue = std::make_shared<TextureQueue>();

	if (!m_pIsolated->CreateDevice(adapter)) {
		return;
	}

	if (FAILED(output->GetDesc(&outputDesc_)))
	{
		Debug::Error("Monitor::Initialize() => IDXGIOutput::GetDesc() failed.");
		return;
	}

	monitorInfo_.cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(outputDesc_.Monitor, &monitorInfo_))
	{
		Debug::Error("Monitor::Initialize() => GetMonitorInfo() failed.");
		return;
	}
	else
	{
		const auto rect = monitorInfo_.rcMonitor;
		width_ = rect.right - rect.left;
		height_ = rect.bottom - rect.top;
	}

	if (FAILED(GetDpiForMonitor(outputDesc_.Monitor, MDT_RAW_DPI, &dpiX_, &dpiY_)))
	{
		Debug::Error("Monitor::Initialize() => GetDpiForMonitor() failed.");
		// DPI is set as -1, so the application has to use the appropriate value.
	}

	ComPtr<IDXGIOutput1> output1;
	if (FAILED(output.As(&output1))) {
		return;
	}

	// use self created device
	auto hr = output1->DuplicateOutput(m_pIsolated->GetDevice().Get(), &deskDupl_);
	switch (hr)
	{
		case S_OK:
		{
			state_ = State::Available;
			const auto rot = static_cast<DXGI_MODE_ROTATION>(GetRotation());
			Debug::Log("Monitor::Initialize() => OK.");
			Debug::Log("    ID    : ", GetId());
			Debug::Log("    Size  : (", GetWidth(), ", ", GetHeight(), ")");
			Debug::Log("    DPI   : (", GetDpiX(), ", ", GetDpiY(), ")");
			Debug::Log("    Rot   : ",
				rot == DXGI_MODE_ROTATION_IDENTITY ? "Landscape" :
				rot == DXGI_MODE_ROTATION_ROTATE90 ? "Portrait" :
				rot == DXGI_MODE_ROTATION_ROTATE180 ? "Landscape (flipped)" :
				rot == DXGI_MODE_ROTATION_ROTATE270 ? "Portrait (flipped)" :
				"Unspecified");
			break;
		}
		case E_INVALIDARG:
		{
			state_ = State::InvalidArg;
			Debug::Error("Monitor::Initialize() => Invalid arguments.");
			break;
		}
		case E_ACCESSDENIED:
		{
			// For example, when the user presses Ctrl + Alt + Delete and the screen
			// switches to admin screen, this error occurs. 
			state_ = State::AccessDenied;
			Debug::Error("Monitor::Initialize() => Access denied.");
			break;
		}
		case DXGI_ERROR_UNSUPPORTED:
		{
			// If the display adapter on the computer is running under the Microsoft Hybrid system,
			// this error occurs.
			state_ = State::Unsupported;
			Debug::Error("Monitor::Initialize() => Unsupported display.");
			break;
		}
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
		{
			// When other application use Desktop Duplication API, this error occurs.
			state_ = State::CurrentlyNotAvailable;
			Debug::Error("Monitor::Initialize() => Currently not available.");
			break;
		}
		case DXGI_ERROR_SESSION_DISCONNECTED:
		{
			state_ = State::SessionDisconnected;
			Debug::Error("Monitor::Initialize() => Session disconnected.");
			break;
		}
		default:
		{
			state_ = State::Unknown;
			Debug::Error("Monitor::Render() => Unknown Error.");
			break;
		}
	}

    // check adapter
    DXGI_ADAPTER_DESC adapterDesc;
    adapter->GetDesc(&adapterDesc);
    const auto unityAdapterLuid = GetUnityAdapterLuid();
    const auto isUnityAdapter =
        adapterDesc.AdapterLuid.HighPart == unityAdapterLuid.HighPart &&
        adapterDesc.AdapterLuid.LowPart  == unityAdapterLuid.LowPart;

	// start desktop duplication thread
	m_stopLoop = false;
	if (isUnityAdapter) {
		m_desktopDuplicationThread = std::thread(std::bind(&Monitor::DuplicateAndCopyLoop, this));
	}
	else {
		m_desktopDuplicationThread = std::thread(std::bind(&Monitor::DuplicateAndMapLoop, this));
	}
}


void Monitor::Render(UINT timeout)
{
    if (!deskDupl_) return;

    HRESULT hr;
    ComPtr<IDXGIResource> resource;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    hr = deskDupl_->AcquireNextFrame(timeout, &frameInfo, &resource);
    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                // If any monitor setting has changed (e.g. monitor size has changed),
                // it is necessary to re-initialize monitors.
                Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST.");
                state_ = State::AccessLost;
                break;
            }
            case DXGI_ERROR_WAIT_TIMEOUT:
            {
                // This often occurs when timeout value is small and it is not problem. 
                // Debug::Log("Monitor::Render() => DXGI_ERROR_WAIT_TIMEOUT.");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL.");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Monitor::Render() => E_INVALIDARG.");
                break;
            }
            default:
            {
                state_ = State::Unknown;
                Debug::Error("Monitor::Render() => Unknown Error.");
                break;
            }
        }
        return;
    }

    ID3D11Texture2D* texture;
    if (FAILED(resource.CopyTo(&texture)))
    {
        Debug::Error("Monitor::Render() => resource.As() failed.");
        return;
    }

    // Get texture
    if (unityTexture_)
    {
        D3D11_TEXTURE2D_DESC srcDesc, dstDesc;
        texture->GetDesc(&srcDesc);
        unityTexture_->GetDesc(&dstDesc);
        if (srcDesc.Width != dstDesc.Width ||
            srcDesc.Height != dstDesc.Height)
        {
            Debug::Error("Monitor::Render() => Texture sizes are defferent.");
            Debug::Error("    Source : (", srcDesc.Width, ", ", srcDesc.Height, ")");
            Debug::Error("    Dest   : (", dstDesc.Width, ", ", dstDesc.Height, ")");
            //Debug::Log("    => Try modifying width/height using reported value from DDA.");
            //width_ = srcDesc.Width;
            //height_ = srcDesc.Height;
            state_ = MonitorState::TextureSizeInconsistent;
            //SendMessageToUnity(Message::TextureSizeChanged);
        }
        else
        {
            ComPtr<ID3D11DeviceContext> context;
            GetDevice()->GetImmediateContext(&context);
            context->CopyResource(unityTexture_, texture);
        }
    }

    UpdateMetadata(frameInfo);

    if (frameInfo.PointerPosition.Visible)
    {
        GetMonitorManager()->SetCursorMonitorId(id_);
    }

    if (GetMonitorManager()->GetCursorMonitorId() == id_)
    {
        UpdateCursor(frameInfo);
    }

    if (UseGetPixels())
    {
        CopyTextureFromGpuToCpu(texture);
    }

    hr = deskDupl_->ReleaseFrame();
    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST.");
                state_ = State::AccessLost;
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL.");
                break;
            }
            default:
            {
                state_ = State::Unknown;
                Debug::Error("Monitor::Render() => Unknown Error.");
                break;
            }
        }
        return;
    }

    hasBeenUpdated_ = true;
}


void Monitor::CopyTextureFromThread()
{
	QueueItem item;
	if (m_textureQueue) {
		item=m_textureQueue->Dequeue();
	}
	auto texture = item.Texture;
	if (!texture) {
		return;
	}
	auto &frameInfo = item.Info;

	// Get texture
	if (unityTexture_)
	{
		D3D11_TEXTURE2D_DESC srcDesc, dstDesc;
		texture->GetDesc(&srcDesc);
		unityTexture_->GetDesc(&dstDesc);
		if (srcDesc.Width != dstDesc.Width ||
			srcDesc.Height != dstDesc.Height)
		{
			Debug::Error("Monitor::Render() => Texture sizes are defferent.");
			Debug::Error("    Source : (", srcDesc.Width, ", ", srcDesc.Height, ")");
			Debug::Error("    Dest   : (", dstDesc.Width, ", ", dstDesc.Height, ")");
			//Debug::Log("    => Try modifying width/height using reported value from DDA.");
			//width_ = srcDesc.Width;
			//height_ = srcDesc.Height;
			state_ = MonitorState::TextureSizeInconsistent;
			//SendMessageToUnity(Message::TextureSizeChanged);
		}
		else
		{
			ComPtr<ID3D11DeviceContext> context;
			GetDevice()->GetImmediateContext(&context);
			context->CopyResource(unityTexture_, texture.Get());
		}
	}

	UpdateMetadata(frameInfo);

	if (frameInfo.PointerPosition.Visible)
	{
		GetMonitorManager()->SetCursorMonitorId(id_);
	}

	if (GetMonitorManager()->GetCursorMonitorId() == id_)
	{
		UpdateCursor(frameInfo);
	}

	if (UseGetPixels())
	{
		CopyTextureFromGpuToCpu(texture.Get());
	}

	hasBeenUpdated_ = true;
}


void Monitor::DuplicateAndCopyLoop()
{
	if (!deskDupl_) return;

	while (!m_stopLoop)
	{
		ComPtr<IDXGIResource> resource;
		DXGI_OUTDUPL_FRAME_INFO frameInfo;

		const auto hr = deskDupl_->AcquireNextFrame(INFINITE, &frameInfo, &resource);
		if (FAILED(hr)) 
        {
			switch (hr)
			{
                case DXGI_ERROR_ACCESS_LOST:
                {
                    // If any monitor setting has changed (e.g. monitor size has changed),
                    // it is necessary to re-initialize monitors.
                    //Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST.");
                    state_ = State::AccessLost;
                    break;
                }
                case DXGI_ERROR_WAIT_TIMEOUT:
                {
                    // This often occurs when timeout value is small and it is not problem. 
                    // Debug::Log("Monitor::Render() => DXGI_ERROR_WAIT_TIMEOUT.");
                    break;
                }
                case DXGI_ERROR_INVALID_CALL:
                {
                    //Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL.");
                    break;
                }
                case E_INVALIDARG:
                {
                    //Debug::Error("Monitor::Render() => E_INVALIDARG.");
                    break;
                }
                default:
                {
                    state_ = State::Unknown;
                    //Debug::Error("Monitor::Render() => Unknown Error.");
                    break;
                }
			}
            continue;
		}

        ScopedReleaser releaser([this] 
        {
            const auto hr = deskDupl_->ReleaseFrame();
            if (FAILED(hr))
            {
                switch (hr)
                {
                    case DXGI_ERROR_ACCESS_LOST:
                    {
                        Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST.");
                        state_ = State::AccessLost;
                        break;
                    }
                    case DXGI_ERROR_INVALID_CALL:
                    {
                        Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL.");
                        break;
                    }
                    default:
                    {
                        state_ = State::Unknown;
                        Debug::Error("Monitor::Render() => Unknown Error.");
                        break;
                    }
                }
            }
        });

        ComPtr<ID3D11Texture2D> texture;
        if (FAILED(resource.As(&texture))) {
            return;
        }

        // copy target
        auto copyTarget = m_pIsolated->CreateCopyTargetWithSharedFlag(texture);
        if (!copyTarget) {
            return;
        }

        // copy
        ComPtr<ID3D11DeviceContext> context;
        m_pIsolated->GetDevice()->GetImmediateContext(&context);
        context->CopyResource(copyTarget.Get(), texture.Get());

        m_textureQueue->Enqueue(QueueItem(copyTarget, frameInfo));
	}
}


void Monitor::DuplicateAndMapLoop()
{
	// not implemented;
}


void Monitor::UpdateCursor(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    auto cursor_ = GetMonitorManager()->GetCursor();
    cursor_->UpdateBuffer(this, frameInfo);
    cursor_->Draw(this);
}


void Monitor::UpdateMetadata(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    metaData_.ExpandIfNeeded(frameInfo.TotalMetadataBufferSize);
    UpdateMoveRects(frameInfo);
    UpdateDirtyRects(frameInfo);
}


void Monitor::UpdateMoveRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    moveRectSize_ = metaData_.Size();

    const auto hr = deskDupl_->GetFrameMoveRects(
        moveRectSize_,
        metaData_.As<DXGI_OUTDUPL_MOVE_RECT>(), 
        &moveRectSize_);

    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST (GetFrameMoveRects()).");
                break;
            }
            case DXGI_ERROR_MORE_DATA:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_MORE_DATA (GetFrameMoveRects()).");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL (GetFrameMoveRects()).");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Monitor::Render() => E_INVALIDARG (GetFrameMoveRects()).");
                break;
            }
            default:
            {
                Debug::Error("Monitor::Render() => Unknown Error (GetFrameMoveRects()).");
                break;
            }
        }
        return;
    }
}


void Monitor::UpdateDirtyRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    dirtyRectSize_ = metaData_.Size() - moveRectSize_;

    const auto hr = deskDupl_->GetFrameDirtyRects(
        dirtyRectSize_,
        metaData_.As<RECT>(moveRectSize_ /* offset */), 
        &dirtyRectSize_);

    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST (GetFrameDirtyRects()).");
                break;
            }
            case DXGI_ERROR_MORE_DATA:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_MORE_DATA (GetFrameDirtyRects()).");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL (GetFrameDirtyRects()).");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Monitor::Render() => E_INVALIDARG (GetFrameDirtyRects()).");
                break;
            }
            default:
            {
                Debug::Error("Monitor::Render() => Unknown Error (GetFrameDirtyRects()).");
                break;
            }
        }
        return;
    }
}


int Monitor::GetId() const
{
    return id_;
}


MonitorState Monitor::GetState() const
{
    return state_;
}


void Monitor::SetUnityTexture(ID3D11Texture2D* texture) 
{ 
    unityTexture_ = texture; 
}


ID3D11Texture2D* Monitor::GetUnityTexture() const
{ 
    return unityTexture_; 
}


IDXGIOutputDuplication* Monitor::GetDeskDupl() 
{ 
    return deskDupl_; 
}


void Monitor::GetName(char* buf, int len) const
{
    strcpy_s(buf, len, monitorInfo_.szDevice);
}


bool Monitor::IsPrimary() const
{
    return monitorInfo_.dwFlags == MONITORINFOF_PRIMARY;
}


bool Monitor::HasBeenUpdated() const
{
    return hasBeenUpdated_;
}


int Monitor::GetLeft() const
{
    return static_cast<int>(outputDesc_.DesktopCoordinates.left);
}


int Monitor::GetRight() const
{
    return static_cast<int>(outputDesc_.DesktopCoordinates.right);
}


int Monitor::GetTop() const
{
    return static_cast<int>(outputDesc_.DesktopCoordinates.top);
}


int Monitor::GetBottom() const
{
    return static_cast<int>(outputDesc_.DesktopCoordinates.bottom);
}


int Monitor::GetRotation() const
{
    return static_cast<int>(outputDesc_.Rotation);
}


int Monitor::GetDpiX() const
{
    return static_cast<int>(dpiX_);
}


int Monitor::GetDpiY() const
{
    return static_cast<int>(dpiY_);
}


int Monitor::GetWidth() const
{
    return width_;
}


int Monitor::GetHeight() const
{
    return height_;
}


int Monitor::GetMoveRectCount() const
{
    return moveRectSize_ / sizeof(DXGI_OUTDUPL_MOVE_RECT);
}


DXGI_OUTDUPL_MOVE_RECT* Monitor::GetMoveRects() const
{
    return metaData_.As<DXGI_OUTDUPL_MOVE_RECT>();
}


int Monitor::GetDirtyRectCount() const
{
    return dirtyRectSize_ / sizeof(RECT);
}


RECT* Monitor::GetDirtyRects() const
{
    return metaData_.As<RECT>(moveRectSize_);
}


void Monitor::UseGetPixels(bool use)
{
    useGetPixels_ = use;
}


bool Monitor::UseGetPixels() const
{
    return useGetPixels_;
}


void Monitor::CopyTextureFromGpuToCpu(ID3D11Texture2D* texture)
{
    const auto monitorRot = static_cast<DXGI_MODE_ROTATION>(GetRotation());
    const auto monitorWidth = GetWidth();
    const auto monitorHeight = GetHeight();
    const auto isVertical = 
        monitorRot == DXGI_MODE_ROTATION_ROTATE90 || 
        monitorRot == DXGI_MODE_ROTATION_ROTATE270;
    const auto desktopImageWidth  = !isVertical ? monitorWidth  : monitorHeight;
    const auto desktopImageHeight = !isVertical ? monitorHeight : monitorWidth;

    if (!textureForGetPixels_)
    {
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = desktopImageWidth;
        desc.Height             = desktopImageHeight;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_STAGING;
        desc.BindFlags          = 0;
        desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags          = 0;

        if (FAILED(GetDevice()->CreateTexture2D(&desc, nullptr, &textureForGetPixels_)))
        {
            Debug::Error("Monitor::CopyTextureFromGpuToCpu() => GetDevice()->CreateTexture2D() failed.");
            return;
        }
    }

    {
        ComPtr<ID3D11DeviceContext> context;
        GetDevice()->GetImmediateContext(&context);
        context->CopyResource(textureForGetPixels_.Get(), texture);
    }

    ComPtr<IDXGISurface> surface;
    if (FAILED(textureForGetPixels_.As(&surface)))
    {
        Debug::Error("Monitor::CopyTextureFromGpuToCpu() => texture.As() failed.");
        return;
    }

    DXGI_MAPPED_RECT mappedSurface;
    if (FAILED(surface->Map(&mappedSurface, DXGI_MAP_READ)))
    {
        Debug::Error("Monitor::CopyTextureFromGpuToCpu() => surface->Map() failed.");
        return;
    }

    const UINT size = desktopImageWidth * desktopImageHeight * sizeof(UINT);
    bufferForGetPixels_.ExpandIfNeeded(size);
    std::memcpy(bufferForGetPixels_.Get(), mappedSurface.pBits, size);

    if (FAILED(surface->Unmap()))
    {
        Debug::Error("Monitor::CopyTextureFromGpuToCpu() => surface->Unmap() failed.");
        return;
    }
}


bool Monitor::GetPixels(BYTE* output, int x, int y, int width, int height)
{
    if (!UseGetPixels())
    {
        Debug::Error("Monitor::GetPixels() => UseGetPixels(true) must have been called when you want to use GetPixels().");
        return false;
    }

    if (!bufferForGetPixels_)
    {
        Debug::Error("Monitor::GetPixels() => CopyTextureFromGpuToCpu() has not been called yet.");
        return false;
    }

    const auto monitorRot = static_cast<DXGI_MODE_ROTATION>(GetRotation());
    const auto monitorWidth = GetWidth();
    const auto monitorHeight = GetHeight();
    const auto isVertical = 
        monitorRot == DXGI_MODE_ROTATION_ROTATE90 || 
        monitorRot == DXGI_MODE_ROTATION_ROTATE270;
    const auto desktopImageWidth  = !isVertical ? monitorWidth  : monitorHeight;
    const auto desktopImageHeight = !isVertical ? monitorHeight : monitorWidth;

    // check area in destop coorinates.
    int left, top, right, bottom;

    switch (monitorRot)
    {
        case DXGI_MODE_ROTATION_ROTATE90:
        {
            left   = y;
            top    = monitorWidth - x - width;
            right  = y + width;
            bottom = monitorWidth - x;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE180:
        {
            left   = monitorWidth - x - width;
            top    = monitorHeight - y - height;
            right  = monitorWidth - x;
            bottom = monitorHeight - y;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE270:
        {
            left   = monitorHeight - y - height;
            top    = x;
            right  = monitorHeight - y;
            bottom = x + width;
            break;
        }
        case DXGI_MODE_ROTATION_IDENTITY:
        case DXGI_MODE_ROTATION_UNSPECIFIED:
        default:
        {
            left   = x;
            top    = y;
            right  = x + width;
            bottom = y + height;
            break;
        }
    }

    if (left   <  0 || 
        top    <  0 || 
        right  >= desktopImageWidth || 
        bottom >= desktopImageHeight)
    {
        Debug::Error("Monitor::GetPixels() => is out of area.");
        Debug::Error(
            "    ",
            "(", left, ", ", top, ")", 
            " ~ (", right, ", ", bottom, ") > ",
            "(", desktopImageWidth, ", ", desktopImageHeight, ")");
        return false;
    }

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            int inRow, inCol;
            switch (monitorRot)
            {
                case DXGI_MODE_ROTATION_ROTATE90:
                    inCol = left + row;
                    inRow = bottom - 1 - col;
                    break;
                case DXGI_MODE_ROTATION_ROTATE180:
                    inCol = right - 1 - col;
                    inRow = bottom - 1 - row;
                    break;
                case DXGI_MODE_ROTATION_ROTATE270:
                    inCol = right - 1 - row;
                    inRow = top + col;
                    break;
                case DXGI_MODE_ROTATION_IDENTITY:
                case DXGI_MODE_ROTATION_UNSPECIFIED:
                default:
                    inCol = left + col;
                    inRow = top + row;
                    break;
            }
            const auto inIndex = 4 * (inRow * desktopImageWidth + inCol);

            const auto outRow = height - 1 - row;
            const auto outCol = col;
            const auto outIndex = 4 * (outRow * width + outCol);


            // BGRA -> RGBA
            output[outIndex + 0] = bufferForGetPixels_[inIndex + 2];
            output[outIndex + 1] = bufferForGetPixels_[inIndex + 1];
            output[outIndex + 2] = bufferForGetPixels_[inIndex + 0];
            output[outIndex + 3] = bufferForGetPixels_[inIndex + 3];
        }
    }

    return true;
}
