#include <d3d11.h>
#include "Common.h"
#include "Cursor.h"
#include "MonitorManager.h"
#include "Monitor.h"


Monitor::Monitor(int id, IDXGIOutput* output)
    : id_(id)
{
    // MEMO: 'output' will be released automatically after this ctor.
    output->GetDesc(&outputDesc_);
    monitorInfo_.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(outputDesc_.Monitor, &monitorInfo_);

    auto output1 = reinterpret_cast<IDXGIOutput1*>(output);
    output1->DuplicateOutput(GetDevice(), &deskDupl_);

    cursor_ = std::make_unique<Cursor>(this);
}


Monitor::~Monitor()
{
    deskDupl_->Release();
}


HRESULT Monitor::Render(UINT timeout)
{
    if (deskDupl_ == nullptr || unityTexture_ == nullptr) return 0;

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

    return 0;
}


int Monitor::GetId() const
{
    return id_;
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