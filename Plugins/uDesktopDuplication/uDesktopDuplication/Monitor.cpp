#include <d3d11.h>
#include "Common.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Monitor.h"


Monitor::Monitor(int id)
    : id_(id)
    , cursor_(std::make_unique<Cursor>(this))
{
}

HRESULT Monitor::Initialize(IDXGIOutput* output)
{
    output->GetDesc(&outputDesc_);
    monitorInfo_.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(outputDesc_.Monitor, &monitorInfo_);

    auto output1 = reinterpret_cast<IDXGIOutput1*>(output);
    const auto hr = output1->DuplicateOutput(GetDevice(), &deskDupl_);

	// TODO: error check
    switch (hr)
    {
        case S_OK:
            available_ = true;
            break;
        case E_INVALIDARG:
            break;
        case E_ACCESSDENIED:
			// For example, when the user presses Ctrl + Alt + Delete and the screen
			// switches to admin screen, this error occurs. 
            // GetMonitorManager()->RequireReinitilization();
            break;
        case DXGI_ERROR_UNSUPPORTED:
            // If the display adapter on the computer is running under the Microsoft Hybrid system,
            // this error occurs.
            break;
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
			// When other application use Desktop Duplication API, this error occurs.
            break;
        case DXGI_ERROR_SESSION_DISCONNECTED:
			break;
    }

    return hr;
}


Monitor::~Monitor()
{
	if (deskDupl_ != nullptr)
	{
		deskDupl_->Release();
	}
}


HRESULT Monitor::Render(UINT timeout)
{
	if (deskDupl_ == nullptr)
	{
		return 0;
	}

	if (unityTexture_ == nullptr) return 0;

    IDXGIResource* resource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    const auto hr = deskDupl_->AcquireNextFrame(timeout, &frameInfo, &resource);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11Texture2D* texture;
    resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));

    ID3D11DeviceContext* context;
    GetDevice()->GetImmediateContext(&context);
    context->CopyResource(unityTexture_, texture);
    context->Release();

    cursor_->Update(frameInfo);

    resource->Release();
    deskDupl_->ReleaseFrame();

    return S_OK;
}


int Monitor::GetId() const
{
    return id_;
}


bool Monitor::IsAvailable() const
{
    return available_;
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


void Monitor::UpdateCursorTexture(ID3D11Texture2D* texture)
{
    cursor_->UpdateTexture(texture);
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