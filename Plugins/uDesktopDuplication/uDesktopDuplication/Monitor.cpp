#include <d3d11.h>
#include <ShellScalingAPI.h>
#include "Debug.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Monitor.h"

using namespace Microsoft::WRL;


Monitor::Monitor(int id)
    : id_(id)
    , cursor_(std::make_unique<Cursor>(this))
{
}


Monitor::~Monitor()
{
    if (deskDupl_) 
    {
        deskDupl_->Release();
        deskDupl_ = nullptr;
    }
}


void Monitor::Initialize(IDXGIOutput* output)
{
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

    auto output1 = reinterpret_cast<IDXGIOutput1*>(output);
    switch (output1->DuplicateOutput(GetDevice().Get(), &deskDupl_))
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
                rot == DXGI_MODE_ROTATION_IDENTITY  ? "Landscape" :
                rot == DXGI_MODE_ROTATION_ROTATE90  ? "Portrait" :
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

    // Get texture
    if (unityTexture_)
    {
        ID3D11Texture2D* texture;
        if (FAILED(resource.CopyTo(&texture)))
        {
            Debug::Error("Monitor::Render() => resource.As() failed.");
            return;
        }

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
    UpdateCursor(frameInfo);

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
}


void Monitor::UpdateCursor(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    cursor_->UpdateBuffer(frameInfo);
    cursor_->UpdateTexture();
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


const std::unique_ptr<Cursor>& Monitor::GetCursor() 
{ 
    return cursor_; 
}


void Monitor::GetCursorTexture(ID3D11Texture2D* texture)
{
    cursor_->GetTexture(texture);
}


void Monitor::GetName(char* buf, int len) const
{
    strcpy_s(buf, len, monitorInfo_.szDevice);
}


bool Monitor::IsPrimary() const
{
    return monitorInfo_.dwFlags == MONITORINFOF_PRIMARY;
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