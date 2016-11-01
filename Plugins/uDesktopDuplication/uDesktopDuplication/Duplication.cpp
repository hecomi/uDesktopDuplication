#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Duplication.h"


Duplication::Duplication(IUnityInterfaces* unity)
    : unity_(unity)
    , device_(unity->Get<IUnityGraphicsD3D11>()->GetDevice())
{
    Initialize();
}


Duplication::~Duplication()
{
    Finalize();
    unity_ = nullptr;
}


void Duplication::Initialize()
{
    Finalize();

    // Get factory
    IDXGIFactory1* factory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));

    // Check all display adapters
    int id = 0;
    IDXGIAdapter1* adapter;
    for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
    {
        // Search the main monitor from all outputs
        IDXGIOutput* output;
        for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
        {
            Monitor monitor;
            monitor.id = id++;

            output->GetDesc(&monitor.outputDesc);
            monitor.monitorInfo.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfo(monitor.outputDesc.Monitor, &monitor.monitorInfo);

            auto output1 = reinterpret_cast<IDXGIOutput1*>(output);
            output1->DuplicateOutput(device_, &monitor.deskDupl);
            output->Release();

            monitors_.push_back(monitor);
        }

        adapter->Release();
    }

    factory->Release();
}


void Duplication::Finalize()
{
    // Release all duplicaitons
    for (auto& monitor : monitors_)
    {
        monitor.deskDupl->Release();
    }
    monitors_.clear();
}


bool Duplication::IsValidId(int id) const
{
    return id >= 0 && id < monitors_.size();
}


void Duplication::OnRender(int id)
{
    errorCode_ = 0;
    errorMessage_ = "";

    if (!IsValidId(id)) return;
    auto& monitor = monitors_[id];

    if (monitor.deskDupl == nullptr || monitor.texture == nullptr) return;

    IDXGIResource* resource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    errorCode_ = monitor.deskDupl->AcquireNextFrame(timeout_, &frameInfo, &resource);
    if (FAILED(errorCode_))
    {
        if (errorCode_ == DXGI_ERROR_ACCESS_LOST)
        {
            Initialize();
            errorMessage_ = "[IDXGIOutputDuplication::AcquireNextFrame()] Access lost.";
        }
        else
        {
            errorMessage_ = "[IDXGIOutputDuplication::AcquireNextFrame()] Maybe timeout.";
        }
        return;
    }

    ID3D11Texture2D* texture;
    resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));

    ID3D11DeviceContext* context;
    device_->GetImmediateContext(&context);
    context->CopyResource(monitor.texture, texture);

    if (!UpdateMouse(frameInfo, monitor))
    {
        errorCode_ = -999;
        errorMessage_ = "[UpdateMouse()] failed.";
    }

    resource->Release();
    monitor.deskDupl->ReleaseFrame();
}


bool Duplication::UpdateMouse(const DXGI_OUTDUPL_FRAME_INFO& frameInfo, Monitor& monitor)
{
    auto& pointer = monitor.pointer;
    pointer.isVisible = frameInfo.PointerPosition.Visible != 0;
    pointer.x = frameInfo.PointerPosition.Position.x;
    pointer.y = frameInfo.PointerPosition.Position.y;

    // Pointer type
    const auto pointerType = pointer.shapeInfo.Type;
    const bool isMono = pointerType == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME;
    const bool isColorMask = pointerType == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR;

    if (pointer.isVisible)
    {
        mouseMonitor_ = monitor.id;
    }

    if (mouseMonitor_ != monitor.id) {
        return true;
    }

    // Increase the buffer size if needed
    if (frameInfo.PointerShapeBufferSize > pointer.apiBufferSize)
    {
        if (pointer.apiBuffer) delete[] pointer.apiBuffer;
        pointer.apiBuffer = new BYTE[frameInfo.PointerShapeBufferSize];
        pointer.apiBufferSize = frameInfo.PointerShapeBufferSize;
    }
    if (!pointer.apiBuffer) return true;

    // Get information about the mouse pointer if needed
    if (frameInfo.PointerShapeBufferSize != 0)
    {
        UINT bufferSize;
        monitor.deskDupl->GetFramePointerShape(
            frameInfo.PointerShapeBufferSize,
            reinterpret_cast<void*>(pointer.apiBuffer),
            &bufferSize,
            &pointer.shapeInfo);
    }

    // Size
    const auto w = pointer.shapeInfo.Width;
    const auto h = pointer.shapeInfo.Height / (isMono ? 2 : 1);
    const auto p = pointer.shapeInfo.Pitch; 

    // Convert the buffer given by API into BGRA32
    const auto bgraBufferSize = w * h * 4;
    if (bgraBufferSize > pointer.bgra32BufferSize)
    {
        if (pointer.bgra32Buffer) delete[] pointer.bgra32Buffer;
        pointer.bgra32Buffer = new BYTE[bgraBufferSize];
        pointer.bgra32BufferSize = bgraBufferSize;
    }
    if (!pointer.bgra32Buffer) return true;

    // If masked, copy the desktop image and merge it with masked image.
    if (isMono || isColorMask)
    {
        HRESULT hr;

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        ID3D11DeviceContext* context;
        device_->GetImmediateContext(&context);

        ID3D11Texture2D* texture = nullptr;
        hr = device_->CreateTexture2D(&desc, nullptr, &texture);
        if (FAILED(hr)) 
        {
            return false;
        }

        D3D11_BOX box;
        box.front = 0;
        box.back = 1;
        box.left = pointer.x;
        box.top = pointer.y;
        box.right = pointer.x + w;
        box.bottom = pointer.y + h;
        context->CopySubresourceRegion(texture, 0, 0, 0, 0, monitor.texture, 0, &box);

        IDXGISurface* surface = nullptr;
        hr = texture->QueryInterface(__uuidof(IDXGISurface), (void**)&surface);
        texture->Release();
        if (FAILED(hr))
        {
            return false;
        }

        DXGI_MAPPED_RECT mappedSurface;
        hr = surface->Map(&mappedSurface, DXGI_MAP_READ);
        if (FAILED(hr))
        {
            surface->Release();
            return false;
        }

        // Finally, get the texture behind the mouse pointer.
        const auto desktop32 = reinterpret_cast<UINT*>(mappedSurface.pBits);
        const UINT desktopPitch = mappedSurface.Pitch / sizeof(UINT);

        // Access RGBA values at the same time
        auto output32 = reinterpret_cast<UINT*>(pointer.bgra32Buffer);

        if (isMono)
        {
            for (UINT row = 0; row < h; ++row) 
            {
                BYTE mask = 0x80;
                for (UINT col = 0; col < w; ++col) 
                {
                    const int i = row * w + col;
                    const BYTE andMask = pointer.apiBuffer[col / 8 + row * p] & mask;
                    const BYTE xorMask = pointer.apiBuffer[col / 8 + (row + h) * p] & mask;
                    const UINT andMask32 = andMask ? 0xFFFFFFFF : 0xFF000000;
                    const UINT xorMask32 = xorMask ? 0x00FFFFFF : 0x00000000;
                    output32[i] = (desktop32[row * desktopPitch + col] & andMask32) ^ xorMask32;
                    mask = (mask == 0x01) ? 0x80 : (mask >> 1);
                }
            }
        }
        else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR
        {
            const auto buffer32 = reinterpret_cast<UINT*>(pointer.apiBuffer);

            for (UINT row = 0; row < h; ++row) 
            {
                for (UINT col = 0; col < w; ++col) 
                {
                    const int i = row * w + col;
                    const int j = row * p / sizeof(UINT) + col;

                    UINT mask = 0xFF000000 & buffer32[j];
                    if (mask)
                    {
                        output32[i] = (desktop32[row * desktopPitch + col] ^ buffer32[j]) | 0xFF000000;
                    }
                    else
                    {
                        output32[i] = buffer32[j] | 0xFF000000;
                    }
                }
            }
        }

        hr = surface->Unmap();
        surface->Release();
        if (FAILED(hr))
        {
            return false;
        }
    }
    else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR
    {
        auto output32 = reinterpret_cast<UINT*>(pointer.bgra32Buffer);
        const auto buffer32 = reinterpret_cast<UINT*>(pointer.apiBuffer);
        for (UINT i = 0; i < w * h; ++i) 
        {
            output32[i] = buffer32[i];
        }
    }

    return true;
}


void Duplication::UpdatePointerTexture(int id, ID3D11Texture2D* ptr)
{
    if (!IsValidId(id)) return;
    const auto& pointer = monitors_[id].pointer;
    if (!pointer.bgra32Buffer) return;
    ID3D11DeviceContext* context;
    device_->GetImmediateContext(&context);
    context->UpdateSubresource(ptr, 0, nullptr, pointer.bgra32Buffer, pointer.shapeInfo.Width * 4, 0);
}


void Duplication::SetTimeout(int timeout)
{
    timeout_ = timeout;
}


void Duplication::SetTexturePtr(int id, void* texture)
{
    if (!IsValidId(id)) return;
    monitors_[id].texture = reinterpret_cast<ID3D11Texture2D*>(texture);
}


int Duplication::GetMonitorCount() const
{
    return static_cast<int>(monitors_.size());
}


void Duplication::GetName(int id, char* buf, int len) const
{
    if (!IsValidId(id)) return;
    strcpy_s(buf, len, monitors_[id].monitorInfo.szDevice);
}


bool Duplication::IsPrimary(int id) const
{
    if (!IsValidId(id)) return false;
    return monitors_[id].monitorInfo.dwFlags == MONITORINFOF_PRIMARY;
}


int Duplication::GetWidth(int id) const
{
    if (!IsValidId(id)) return -1;
    const auto rect = monitors_[id].monitorInfo.rcMonitor;
    return rect.right - rect.left;
}


int Duplication::GetHeight(int id) const
{
    if (!IsValidId(id)) return -1;
    const auto rect = monitors_[id].monitorInfo.rcMonitor;
    return rect.bottom - rect.top;
}


int Duplication::IsPointerVisible(int id) const
{
    if (!IsValidId(id)) return false;
    return monitors_[id].pointer.isVisible;
}


int Duplication::GetPointerX(int id) const
{
    if (!IsValidId(id)) return -1;
    return monitors_[id].pointer.x;
}


int Duplication::GetPointerY(int id) const
{
    if (!IsValidId(id)) return -1;
    return monitors_[id].pointer.y;
}


int Duplication::GetPointerShapeWidth(int id) const
{
    if (!IsValidId(id)) return -1;
    return monitors_[id].pointer.shapeInfo.Width;
}


int Duplication::GetPointerShapeHeight(int id) const
{
    if (!IsValidId(id)) return -1;
    const auto& info = monitors_[id].pointer.shapeInfo;
    return (info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) ? info.Height / 2 : info.Height;
}


int Duplication::GetPointerShapePitch(int id) const
{
    if (!IsValidId(id)) return -1;
    return monitors_[id].pointer.shapeInfo.Pitch;
}


int Duplication::GetPointerShapeType(int id) const
{
    if (!IsValidId(id)) return -1;
    return monitors_[id].pointer.shapeInfo.Type;
}


int Duplication::GetErrorCode() const
{
    return errorCode_;
}


void Duplication::GetErrorMessage(char* buf, int len) const
{
    strcpy_s(buf, len, errorMessage_.c_str());
}