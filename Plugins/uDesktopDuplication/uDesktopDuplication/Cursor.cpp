#include <d3d11.h>
#include "Common.h"
#include "Debug.h"
#include "MonitorManager.h"
#include "Monitor.h"
#include "Cursor.h"


Cursor::Cursor(Monitor* monitor)
    : monitor_(monitor)
{
}


Cursor::~Cursor()
{
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

    if (!IsCursorOnParentMonitor())
    {
        return;
    }

    if (frameInfo.PointerShapeBufferSize == 0)
    {
        return;
    }

    // Increase the buffer size if needed
    if (frameInfo.PointerShapeBufferSize > apiBufferSize_)
    {
        apiBufferSize_ = frameInfo.PointerShapeBufferSize;
        apiBuffer_ = std::make_unique<BYTE[]>(apiBufferSize_);
    }
    if (!apiBuffer_) return;

    // Get mouse pointer information
    UINT bufferSize;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;
    const auto hr = monitor_->GetDeskDupl()->GetFramePointerShape(
        apiBufferSize_,
        reinterpret_cast<void*>(apiBuffer_.get()),
        &bufferSize,
        &shapeInfo);

    if (FAILED(hr))
    {
        Debug::Error("Cursor::UpdateBuffer() => GetFramePointerShape() failed.");
        apiBuffer_.reset();
        apiBufferSize_ = 0;
    }

    shapeInfo_ = shapeInfo;
}


void Cursor::UpdateTexture()
{
    if (!IsCursorOnParentMonitor())
    {
        return;
    }

    // cursor type
    const bool isMono = GetType() == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME;
    const bool isColorMask = GetType() == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR;

    // Size
    const auto w0 = GetWidth();
    const auto h0 = GetHeight();
    const auto p = GetPitch();
    auto w = w0;
    auto h = h0;

    // Convert the buffer given by API into BGRA32
    const UINT bgraBufferSize = w0 * h0 * 4;
    if (bgraBufferSize > bgra32BufferSize_)
    {
        bgra32BufferSize_ = bgraBufferSize;
        bgra32Buffer_ = std::make_unique<BYTE[]>(bgra32BufferSize_);
    }
    if (!bgra32Buffer_) return;

    // If masked, copy the desktop image and merge it with masked image.
    if (isMono || isColorMask)
    {
        HRESULT hr;

        const auto mw = monitor_->GetWidth();
        const auto mh = monitor_->GetHeight();
        auto x = x_;
        auto y = y_;
        auto colMin = 0;
        auto colMax = w0;
        auto rowMin = 0;
        auto rowMax = h0;

        if (x < 0)
        {
            x = 0;
            w = w0 + x_;
            colMin = w0 - w;
        }
        if (x + w >= mw) 
        {
            w = mw - x_;
            colMax = w;
        }
        if (y < 0)
        {
            y = 0;
            h = h0 + y_;
            rowMin = h0 - h;
        }
        if (y + h >= mh) 
        {
            h = mh - y_;
            rowMax = h;
        }

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
        const auto textureReleaser = MakeUniqueWithReleaser(texture);
        if (FAILED(hr)) 
        {
            Debug::Error("Cursor::UpdateTexture() => GetDevice()->CreateTexture2D() failed.");
            return;
        }

        D3D11_BOX box;
        box.front = 0;
        box.back = 1;
        box.left = x;
        box.top = y;
        box.right = x + w;
        box.bottom = y + h;

        if (monitor_->GetUnityTexture() == nullptr) 
        {
            Debug::Error("Cursor::UpdateTexture() => Monitor::GetUnityTexture() is null.");
            return;
        }

        ID3D11DeviceContext* context;
        GetDevice()->GetImmediateContext(&context);
        context->CopySubresourceRegion(texture, 0, 0, 0, 0, monitor_->GetUnityTexture(), 0, &box);
        context->Release();

        IDXGISurface* surface = nullptr;
        hr = texture->QueryInterface(__uuidof(IDXGISurface), (void**)&surface);
        const auto surfaceReleaser = MakeUniqueWithReleaser(surface);
        if (FAILED(hr))
        {
            Debug::Error("Cursor::UpdateTexture() => texture->QueryInterface() failed.");
            return;
        }

        DXGI_MAPPED_RECT mappedSurface;
        hr = surface->Map(&mappedSurface, DXGI_MAP_READ);
        if (FAILED(hr))
        {
            Debug::Error("Cursor::UpdateTexture() => surface->Map() failed.");
            return;
        }

        // Finally, get the texture behind the mouse cursor.
        const auto desktop32 = reinterpret_cast<UINT*>(mappedSurface.pBits);
        const UINT desktopPitch = mappedSurface.Pitch / sizeof(UINT);

        // Access RGBA values at the same time
        auto output32 = reinterpret_cast<UINT*>(bgra32Buffer_.get());

        if (isMono)
        {
            for (int row = rowMin, y = 0; row < rowMax; ++row, ++y) 
            {
                for (int col = colMin, x = 0; col < colMax; ++col, ++x) 
                {
                    BYTE mask = 0b10000000 >> (col % 8);
                    const int i = row * w0 + col;
                    const BYTE andMask = apiBuffer_[col / 8 + row * p] & mask;
                    const BYTE xorMask = apiBuffer_[col / 8 + (row + h) * p] & mask;
                    const UINT andMask32 = andMask ? 0xFFFFFFFF : 0x00000000;
                    const UINT xorMask32 = xorMask ? 0xFFFFFFFF : 0x00000000;
                    output32[i] = (desktop32[y * desktopPitch + x] & andMask32) ^ xorMask32;
                }
            }
        }
        else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR
        {
            const auto buffer32 = reinterpret_cast<UINT*>(apiBuffer_.get());

            for (int row = rowMin, y = 0; row < rowMax; ++row, ++y) 
            {
                for (int col = colMin, x = 0; col < colMax; ++col, ++x) 
                {
                    const int i = col + row * w0;
                    const int j = col + row * p / sizeof(UINT);

                    UINT mask = 0xFF000000 & buffer32[j];
                    if (mask)
                    {
                        output32[i] = (desktop32[y * desktopPitch + x] ^ buffer32[j]) | 0xFF000000;
                    }
                    else
                    {
                        output32[i] = buffer32[j] | 0xFF000000;
                    }
                }
            }
        }

        hr = surface->Unmap();
        if (FAILED(hr))
        {
            return;
        }
    }
    else // DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR
    {
        auto output32 = reinterpret_cast<UINT*>(bgra32Buffer_.get());
        const auto buffer32 = reinterpret_cast<UINT*>(apiBuffer_.get());
        for (int i = 0; i < w * h; ++i) 
        {
            output32[i] = buffer32[i];
        }
    }
}


void Cursor::GetTexture(ID3D11Texture2D* texture)
{
    if (bgra32Buffer_ == nullptr)
    {
        Debug::Error("Cursor::GetTexture() => bgra32Buffer is null.");
        return;
    }

    if (texture == nullptr)
    {
        Debug::Error("Cursor::GetTexture() => The given texture is null.");
        return;
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    if ((int)desc.Width < GetWidth() || (int)desc.Height < GetHeight())
    {
        char buf[256];
        sprintf_s(buf, 256,
            "Cursor::GetTexture() => The given texture has smaller width / height.\n"
            "Given => (%d, %d)  Buffer => (%d, %d)",
            desc.Width, desc.Height, GetWidth(), GetHeight());
        Debug::Error(buf);
        return;
    }

    ID3D11DeviceContext* context;
    GetDevice()->GetImmediateContext(&context);
    context->UpdateSubresource(texture, 0, nullptr, bgra32Buffer_.get(), GetWidth() * 4, 0);
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


bool Cursor::IsCursorOnParentMonitor() const
{
    return GetMonitorManager()->GetCursorMonitorId() == monitor_->GetId();
}