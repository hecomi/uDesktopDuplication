#include <d3d11.h>
#include <ShellScalingAPI.h>
#include <queue>
#include "Monitor.h"
#include "Duplicator.h"
#include "Debug.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Device.h"

using namespace Microsoft::WRL;



Monitor::Monitor(int id)
    : id_(id)
{
	ZeroMemory(&monitorInfo_, sizeof(monitorInfo_));
}


Monitor::~Monitor()
{
}


void Monitor::Initialize(
	const ComPtr<IDXGIAdapter> &adapter,
    const ComPtr<IDXGIOutput> &output
)
{
    UDD_FUNCTION_SCOPE_TIMER

    adapter_ = adapter;
    output_ = output;

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

    const auto rot = outputDesc_.Rotation;
    Debug::Log("Monitor::Initialized() =>");
    Debug::Log("    ID    : ", id_);
    Debug::Log("    Size  : (", width_, ", ", height_, ")");
    Debug::Log("    DPI   : (", dpiX_, ", ", dpiY_, ")");
    Debug::Log("    Rot   : ",
        rot == DXGI_MODE_ROTATION_IDENTITY ? "Landscape" :
        rot == DXGI_MODE_ROTATION_ROTATE90 ? "Portrait" :
        rot == DXGI_MODE_ROTATION_ROTATE180 ? "Landscape (flipped)" :
        rot == DXGI_MODE_ROTATION_ROTATE270 ? "Portrait (flipped)" :
        "Unspecified");

    duplicator_ = std::make_shared<Duplicator>(this);
}


void Monitor::Finalize()
{
    UDD_FUNCTION_SCOPE_TIMER

    StopCapture();
}


void Monitor::Render()
{
    UDD_FUNCTION_SCOPE_TIMER

    const auto& frame = duplicator_->GetLastFrame();

    if (frame.id == lastFrameId_) return;
    lastFrameId_ = frame.id;

    if (unityTexture_ == nullptr) 
    {
        Debug::Error("Monitor::Render() => Target texture has not been set yet.");
        return;
    }

    if (!frame.texture)
    {
        Debug::Error("Monitor::Render() => frame doesn't have texture.");
        return;
    }

    D3D11_TEXTURE2D_DESC srcDesc, dstDesc;
    frame.texture->GetDesc(&srcDesc);
    unityTexture_->GetDesc(&dstDesc);
    if (srcDesc.Width  != dstDesc.Width ||
        srcDesc.Height != dstDesc.Height)
    {
        Debug::Error("Monitor::Render() => Texture sizes are defferent.");
        Debug::Error("    Source : (", srcDesc.Width, ", ", srcDesc.Height, ")");
        Debug::Error("    Dest   : (", dstDesc.Width, ", ", dstDesc.Height, ")");
        return;
    }
    else
    {
        ComPtr<ID3D11DeviceContext> context;
        GetDevice()->GetImmediateContext(&context);
        context->CopyResource(unityTexture_, frame.texture.Get());

        auto& manager = GetMonitorManager();
        if (id_ == manager->GetCursorMonitorId())
        {
            manager->GetCursor()->Draw(unityTexture_);
        }
    }

	if (UseGetPixels())
	{
		CopyTextureFromGpuToCpu(unityTexture_);
	}

	hasBeenUpdated_ = true;
}


void Monitor::StartCapture()
{
    UDD_FUNCTION_SCOPE_TIMER

    if (duplicator_->GetState() == DuplicatorState::Ready)
    {
        duplicator_->Start();
    }
}


void Monitor::StopCapture()
{
    UDD_FUNCTION_SCOPE_TIMER

    duplicator_->Stop();
}


int Monitor::GetId() const
{
    return id_;
}


ComPtr<struct IDXGIAdapter> Monitor::GetAdapter()
{
    return adapter_;
}


ComPtr<struct IDXGIOutput> Monitor::GetOutput()
{
    return output_;
}


DuplicatorState Monitor::GetDuplicatorState() const
{
    return duplicator_->GetState();
}


void Monitor::SetUnityTexture(ID3D11Texture2D* texture) 
{ 
    unityTexture_ = texture; 
}


ID3D11Texture2D* Monitor::GetUnityTexture() const
{ 
    return unityTexture_; 
}


ComPtr<IDXGIOutputDuplication> Monitor::GetDeskDupl() 
{ 
    return duplicator_->GetDuplication(); 
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
    const auto& metaData = duplicator_->GetLastFrame().metaData;
    return metaData.moveRectSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);
}


DXGI_OUTDUPL_MOVE_RECT* Monitor::GetMoveRects() const
{
    const auto& metaData = duplicator_->GetLastFrame().metaData;
    return metaData.buffer.As<DXGI_OUTDUPL_MOVE_RECT>();
}


int Monitor::GetDirtyRectCount() const
{
    const auto& metaData = duplicator_->GetLastFrame().metaData;
    return metaData.dirtyRectSize / sizeof(RECT);
}


RECT* Monitor::GetDirtyRects() const
{
    const auto& metaData = duplicator_->GetLastFrame().metaData;
    return metaData.buffer.As<RECT>(metaData.moveRectSize);
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
    UDD_FUNCTION_SCOPE_TIMER

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
    UDD_FUNCTION_SCOPE_TIMER

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
