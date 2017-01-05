#include "Monitor.h"
#include "MonitorManager.h"
#include "Debug.h"
#include <ShellScalingApi.h>
#include <queue>
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


void Monitor::InitializeThreaded(
	const ComPtr<IDXGIAdapter> &adapter
	, bool isUnityDeviceAdapter
	, const ComPtr<IDXGIOutput> &output
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

	// start desktop duplication thread
	m_stopLoop = false;
	if (isUnityDeviceAdapter) {
		m_desktopDuplicationThread = std::thread(std::bind(&Monitor::DuplicateAndCopyLoop, this));
	}
	else {
		m_desktopDuplicationThread = std::thread(std::bind(&Monitor::DuplicateAndMapLoop, this));
	}
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
	if (!deskDupl_)return;

	while (!m_stopLoop)
	{
		ComPtr<IDXGIResource> resource;
		DXGI_OUTDUPL_FRAME_INFO frameInfo;
		auto hr = deskDupl_->AcquireNextFrame(INFINITE, &frameInfo, &resource);

		if (FAILED(hr)) {
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
		}
		else
		{
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
}

void Monitor::DuplicateAndMapLoop()
{
	// not implemented;
}
