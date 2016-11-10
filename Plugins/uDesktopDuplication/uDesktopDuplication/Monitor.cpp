#include <d3d11.h>
#include <ShellScalingAPI.h>
#include "Common.h"
#include "Debug.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Monitor.h"


Monitor::Monitor(int id)
    : id_(id)
    , cursor_(std::make_unique<Cursor>(this))
{
}


Monitor::~Monitor()
{
}


void Monitor::Initialize(IDXGIOutput* output)
{
    output->GetDesc(&outputDesc_);
    monitorInfo_.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(outputDesc_.Monitor, &monitorInfo_);

    GetDpiForMonitor(outputDesc_.Monitor, MDT_RAW_DPI, &dpiX_, &dpiY_);

    auto output1 = reinterpret_cast<IDXGIOutput1*>(output);
    IDXGIOutputDuplication* deskDupl;
    const auto hr = output1->DuplicateOutput(GetDevice(), &deskDupl);

    // TODO: error check
    switch (hr)
    {
        case S_OK:
            state_ = State::Available;
            deskDupl_ = std::shared_ptr<IDXGIOutputDuplication>(
                deskDupl,
                [](IDXGIOutputDuplication* ptr) { if (ptr != nullptr) ptr->Release(); });
            break;
        case E_INVALIDARG:
            state_ = State::InvalidArg;
            Debug::Error("Monitor::Initialize() => Invalid arguments.");
            break;
        case E_ACCESSDENIED:
            // For example, when the user presses Ctrl + Alt + Delete and the screen
            // switches to admin screen, this error occurs. 
            state_ = State::AccessDenied;
            Debug::Error("Monitor::Initialize() => Access denied.");
            break;
        case DXGI_ERROR_UNSUPPORTED:
            // If the display adapter on the computer is running under the Microsoft Hybrid system,
            // this error occurs.
            state_ = State::Unsupported;
            Debug::Error("Monitor::Initialize() => Unsupported display.");
            break;
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
            // When other application use Desktop Duplication API, this error occurs.
            state_ = State::CurrentlyNotAvailable;
            Debug::Error("Monitor::Initialize() => Currently not available.");
            break;
        case DXGI_ERROR_SESSION_DISCONNECTED:
            state_ = State::SessionDisconnected;
            Debug::Error("Monitor::Initialize() => Session disconnected.");
            break;
    }
}


void Monitor::Render(UINT timeout)
{
    if (!deskDupl_) return;

    IDXGIResource* resource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr;

    hr = deskDupl_->AcquireNextFrame(timeout, &frameInfo, &resource);
    const auto resourceReleaser = MakeUniqueWithReleaser(resource);
    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
                // If any monitor setting has changed (e.g. monitor size has changed),
                // it is necessary to re-initialize monitors.
                Debug::Log("Monitor::Render() => DXGI_ERROR_ACCESS_LOST.");
                state_ = State::AccessLost;
                break;
            case DXGI_ERROR_WAIT_TIMEOUT:
                // This often occurs when timeout value is small and it is not problem. 
                // Debug::Log("Monitor::Render() => DXGI_ERROR_WAIT_TIMEOUT.");
                break;
            case DXGI_ERROR_INVALID_CALL:
                Debug::Error("Monitor::Render() => DXGI_ERROR_INVALID_CALL.");
                break;
            case E_INVALIDARG:
                Debug::Error("Monitor::Render() => E_INVALIDARG.");
                break;
            default:
                break;
        }
        return;
    }

    if (unityTexture_)
    {
        ID3D11Texture2D* texture;
        resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));

        ID3D11DeviceContext* context;
        GetDevice()->GetImmediateContext(&context);
        context->CopyResource(unityTexture_, texture);
        context->Release();
    }

    cursor_->UpdateBuffer(frameInfo);
    cursor_->UpdateTexture();

    hr = deskDupl_->ReleaseFrame();
    if (FAILED(hr))
    {
        Debug::Error("Monitor::Render() => ReleaseFrame() failed.");
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


const std::shared_ptr<IDXGIOutputDuplication>& Monitor::GetDeskDupl() 
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
    return outputDesc_.DesktopCoordinates.left;
}


int Monitor::GetRight() const
{
    return outputDesc_.DesktopCoordinates.right;
}


int Monitor::GetTop() const
{
    return outputDesc_.DesktopCoordinates.top;
}


int Monitor::GetBottom() const
{
    return outputDesc_.DesktopCoordinates.bottom;
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
    const auto rect = monitorInfo_.rcMonitor;
    return rect.right - rect.left;
}


int Monitor::GetHeight() const
{
    const auto rect = monitorInfo_.rcMonitor;
    return rect.bottom - rect.top;
}