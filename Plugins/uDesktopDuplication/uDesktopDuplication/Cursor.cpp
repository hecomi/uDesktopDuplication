#include <d3d11.h>
#include "Common.h"
#include "MonitorManager.h"
#include "Monitor.h"
#include "Cursor.h"


Cursor::Cursor(Monitor* monitor)
    : monitor_(monitor)
{
}


Cursor::~Cursor()
{
    if (apiBuffer_ != nullptr)
    {
        delete[] apiBuffer_;
        apiBuffer_ = nullptr;
    }
    if (bgra32Buffer_ != nullptr)
    {
        delete[] bgra32Buffer_;
        bgra32Buffer_ = nullptr;
    }
}


void Cursor::UpdateBuffer(const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    if (frameInfo.LastMouseUpdateTime.QuadPart == 0)
    {
        return;
    }

    x_ = frameInfo.PointerPosition.Position.x;
    y_ = frameInfo.PointerPosition.Position.y;
    isVisible_ = frameInfo.PointerPosition.Visible != 0;
    timestamp_ = frameInfo.LastMouseUpdateTime;

    if (isVisible_) 
    {
        GetMonitorManager()->SetCursorMonitorId(monitor_->GetId());
    }

    if (frameInfo.PointerShapeBufferSize == 0)
    {
        return;
    }

    // Increase the buffer size if needed
    if (frameInfo.PointerShapeBufferSize > apiBufferSize_)
    {
        if (apiBuffer_) delete[] apiBuffer_;
        apiBuffer_ = new BYTE[frameInfo.PointerShapeBufferSize];
        apiBufferSize_ = frameInfo.PointerShapeBufferSize;
    }
    if (apiBuffer_ == nullptr) return;

    // Get mouse pointer information
    UINT bufferSize;
    const auto hr = monitor_->GetDeskDupl()->GetFramePointerShape(
        frameInfo.PointerShapeBufferSize,
        reinterpret_cast<void*>(apiBuffer_),
        &bufferSize,
        &shapeInfo_);
    if (FAILED(hr))
    {
        delete[] apiBuffer_;
        apiBuffer_ = nullptr;
        apiBufferSize_ = 0;
    }
}


void Cursor::UpdateTexture()
{
    // cursor type
    const bool isMono = GetType() == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME;
    const bool isColorMask = GetType() == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR;

    // Size
    const auto w = GetWidth();
    const auto h = GetHeight();
    const auto p = GetPitch();

    // Convert the buffer given by API into BGRA32
    const UINT bgraBufferSize = w * h * 4;
    if (bgraBufferSize > bgra32BufferSize_)
    {
        if (bgra32Buffer_) delete[] bgra32Buffer_;
        bgra32Buffer_ = new BYTE[bgraBufferSize];
        bgra32BufferSize_ = bgraBufferSize;
    }
    if (bgra32Buffer_ == nullptr) return;

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

        ID3D11Texture2D* texture = nullptr;
        hr = GetDevice()->CreateTexture2D(&desc, nullptr, &texture);
        if (FAILED(hr)) 
        {
            return;
        }

        D3D11_BOX box;
        box.front = 0;
        box.back = 1;
        box.left = x_;
        box.top = y_;
        box.right = x_ + w;
        box.bottom = y_ + h;

        ID3D11DeviceContext* context;
        GetDevice()->GetImmediateContext(&context);
        context->CopySubresourceRegion(texture, 0, 0, 0, 0, monitor_->GetUnityTexture(), 0, &box);
        context->Release();

        IDXGISurface* surface = nullptr;
        hr = texture->QueryInterface(__uuidof(IDXGISurface), (void**)&surface);
        texture->Release();
        if (FAILED(hr))
        {
            return;
        }

        DXGI_MAPPED_RECT mappedSurface;
        hr = surface->Map(&mappedSurface, DXGI_MAP_READ);
        if (FAILED(hr))
        {
            surface->Release();
            return;
        }

        // Finally, get the texture behind the mouse cursor.
        const auto desktop32 = reinterpret_cast<UINT*>(mappedSurface.pBits);
        const UINT desktopPitch = mappedSurface.Pitch / sizeof(UINT);

        // Access RGBA values at the same time
        auto output32 = reinterpret_cast<UINT*>(bgra32Buffer_);

        if (isMono)
        {
            for (int row = 0; row < h; ++row) 
            {
                BYTE mask = 0x80;
                for (int col = 0; col < w; ++col) 
                {
                    const int i = row * w + col;
                    const BYTE andMask = apiBuffer_[col / 8 + row * p] & mask;
                    const BYTE xorMask = apiBuffer_[col / 8 + (row + h) * p] & mask;
                    const UINT andMask32 = andMask ? 0xFFFFFFFF : 0xFF000000;
                    const UINT xorMask32 = xorMask ? 0x00FFFFFF : 0x00000000;
                    output32[i] = (desktop32[row * desktopPitch + col] & andMask32) ^ xorMask32;
                    mask = (mask == 0x01) ? 0x80 : (mask >> 1);
                }
            }
        }
        else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR
        {
            const auto buffer32 = reinterpret_cast<UINT*>(apiBuffer_);

            for (int row = 0; row < h; ++row) 
            {
                for (int col = 0; col < w; ++col) 
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
            return;
        }
    }
    else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR
    {
        auto output32 = reinterpret_cast<UINT*>(bgra32Buffer_);
        const auto buffer32 = reinterpret_cast<UINT*>(apiBuffer_);
        for (int i = 0; i < w * h; ++i) 
        {
            output32[i] = buffer32[i];
        }
    }

    return;
}


void Cursor::GetTexture(ID3D11Texture2D* texture)
{
    if (bgra32Buffer_ == nullptr) return;
    ID3D11DeviceContext* context;
    GetDevice()->GetImmediateContext(&context);
    context->UpdateSubresource(texture, 0, nullptr, bgra32Buffer_, shapeInfo_.Width * 4, 0);
    context->Release();
}


bool Cursor::IsVisible() const 
{ 
    return isVisible_; 
}


int Cursor::GetX() const 
{ 
    return x_; 
}


int Cursor::GetY() const 
{ 
    return y_; 
}


int Cursor::GetWidth() const
{
    return shapeInfo_.Width;
}


int Cursor::GetHeight() const
{
    return (shapeInfo_.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) ? 
        shapeInfo_.Height / 2 : 
        shapeInfo_.Height;
}


int Cursor::GetPitch() const
{
    return shapeInfo_.Pitch;
}


int Cursor::GetType() const
{
    return shapeInfo_.Type;
}
