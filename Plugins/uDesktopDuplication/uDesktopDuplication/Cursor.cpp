#include <d3d11.h>
#include <wrl/client.h>
#include "Debug.h"
#include "MonitorManager.h"
#include "Monitor.h"
#include "Cursor.h"

using namespace Microsoft::WRL;


Cursor::Cursor()
{
}


Cursor::~Cursor()
{
}


void Cursor::UpdateBuffer(Monitor* monitor, const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    if (frameInfo.LastMouseUpdateTime.QuadPart == 0)
    {
        return;
    }

    isVisible_ = frameInfo.PointerPosition.Visible != 0;
    if (isVisible_) 
    {
        GetMonitorManager()->SetCursorMonitorId(monitor->GetId());
    }

    x_ = frameInfo.PointerPosition.Position.x;
    y_ = frameInfo.PointerPosition.Position.y;
    timestamp_ = frameInfo.LastMouseUpdateTime;

    if (frameInfo.PointerShapeBufferSize == 0)
    {
        return;
    }

	apiBuffer_.ExpandIfNeeded(frameInfo.PointerShapeBufferSize);
    if (!apiBuffer_) 
    {
        return;
    }

    // Get mouse pointer information
    UINT bufferSize;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;
    const auto hr = monitor->GetDeskDupl()->GetFramePointerShape(
        apiBuffer_.Size(),
        apiBuffer_.Get(),
        &bufferSize,
        &shapeInfo);

    if (FAILED(hr))
    {
        Debug::Error("Cursor::UpdateBuffer() => GetFramePointerShape() failed.");
        apiBuffer_.Reset();
        return;
    }

    shapeInfo_ = shapeInfo;
}


void Cursor::Draw(Monitor* monitor)
{
    // Check desktop texure
    if (monitor->GetUnityTexture() == nullptr) 
    {
        Debug::Error("Cursor::UpdateTexture() => Monitor::GetUnityTexture() is null.");
        return;
    }

    // Cursor information
    const auto cursorImageWidth = GetWidth();
    const auto cursorImageHeight = GetHeight();
    const auto cursorImagePitch = GetPitch();

    // Captured size
    auto capturedImageWidth = cursorImageWidth;
    auto capturedImageHeight = cursorImageHeight;

    // Convert the buffer given by API into BGRA32
    const UINT bgraBufferSize = cursorImageWidth * cursorImageHeight * 4;
	bgraBuffer_.ExpandIfNeeded(bgraBufferSize);
    
    // Check buffers
    if (!bgraBuffer_ || !apiBuffer_) 
    {
        return;
    }

    // Calculate information to capture desktop image under cursor.
    D3D11_BOX box;
    const auto monitorRot = static_cast<DXGI_MODE_ROTATION>(monitor->GetRotation());
    auto colMin = 0;
    auto colMax = cursorImageWidth;
    auto rowMin = 0;
    auto rowMax = cursorImageHeight;

    {
        const auto monitorWidth = monitor->GetWidth();
        const auto monitorHeight = monitor->GetHeight();
        const auto isVertical = 
            monitorRot == DXGI_MODE_ROTATION_ROTATE90 || 
            monitorRot == DXGI_MODE_ROTATION_ROTATE270;
        const auto desktopImageWidth  = !isVertical ? monitorWidth  : monitorHeight;
        const auto desktopImageHeight = !isVertical ? monitorHeight : monitorWidth;

        auto x = x_;
        auto y = y_;

        if (x < 0)
        {
            x = 0;
            capturedImageWidth = cursorImageWidth + x_;
            colMin = cursorImageWidth - capturedImageWidth;
        }
        if (x + capturedImageWidth >= monitorWidth) 
        {
            capturedImageWidth = monitorWidth - x_;
            colMax = capturedImageWidth;
        }
        if (y < 0)
        {
            y = 0;
            capturedImageHeight = cursorImageHeight + y_;
            rowMin = cursorImageHeight - capturedImageHeight;
        }
        if (y + capturedImageHeight >= monitorHeight) 
        {
            capturedImageHeight = monitorHeight - y_;
            rowMax = capturedImageHeight;
        }

        box.front = 0;
        box.back = 1;

        switch (monitorRot)
        {
            case DXGI_MODE_ROTATION_ROTATE90:
            {
                box.left   = y;
                box.top    = monitorWidth - x - capturedImageWidth;
                box.right  = y + capturedImageWidth;
                box.bottom = monitorWidth - x;
                break;
            }
            case DXGI_MODE_ROTATION_ROTATE180:
            {
                box.left   = monitorWidth - x - capturedImageWidth;
                box.top    = monitorHeight - y - capturedImageHeight;
                box.right  = monitorWidth - x;
                box.bottom = monitorHeight - y;
                break;
            }
            case DXGI_MODE_ROTATION_ROTATE270:
            {
                box.left   = monitorHeight - y - capturedImageHeight;
                box.top    = x;
                box.right  = monitorHeight - y;
                box.bottom = x + capturedImageWidth;
                break;
            }
            case DXGI_MODE_ROTATION_IDENTITY:
            case DXGI_MODE_ROTATION_UNSPECIFIED:
            {
                box.left   = x;
                box.top    = y;
                box.right  = x + capturedImageWidth;
                box.bottom = y + capturedImageHeight;
                break;
            }
        }

        if (box.left   <  0 || 
            box.top    <  0 || 
            box.right  > static_cast<UINT>(desktopImageWidth) || 
            box.bottom > static_cast<UINT>(desktopImageHeight))
        {
            Debug::Error("Cursor::UpdateTexture() => box is out of area.");
            Debug::Error(
                "    ",
                "(", box.left, ", ", box.top, ")", 
                " ~ (", box.right, ", ", box.bottom, ") > ",
                "(", desktopImageWidth, ", ", desktopImageHeight, ")");
            return;
        }
    }

    // Create texture
    ComPtr<ID3D11Texture2D> texture;
    {
        D3D11_TEXTURE2D_DESC desc;
        desc.Width = capturedImageWidth;
        desc.Height = capturedImageHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        if (FAILED(GetDevice()->CreateTexture2D(&desc, nullptr, &texture)))
        {
            Debug::Error("Cursor::UpdateTexture() => GetDevice()->CreateTexture2D() failed.");
            return;
        }
    }

    // Copy desktop image to the texture
    {
        ComPtr<ID3D11DeviceContext> context;
        GetDevice()->GetImmediateContext(&context);
        context->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0, monitor->GetUnityTexture(), 0, &box);
    }

    // Get mapped surface
    ComPtr<IDXGISurface> surface;
    if (FAILED(texture.As(&surface)))
    {
        Debug::Error("Cursor::UpdateTexture() => texture->QueryInterface() failed.");
        return;
    }

    DXGI_MAPPED_RECT mappedSurface;
    if (FAILED(surface->Map(&mappedSurface, DXGI_MAP_READ)))
    {
        Debug::Error("Cursor::UpdateTexture() => surface->Map() failed.");
        return;
    }

    // Finally, get the desktop texture under the mouse cursor.
    const auto desktop32 = reinterpret_cast<UINT*>(mappedSurface.pBits);
    const UINT desktopPitch = mappedSurface.Pitch / sizeof(UINT);

    // Take the monitor orientation into consideration.
    const auto getDesktop32 = [&](int col, int row)
    {
        switch (monitorRot)
        {
            case DXGI_MODE_ROTATION_ROTATE90:
                return desktop32[(capturedImageWidth - 1 - col) * desktopPitch + row];
            case DXGI_MODE_ROTATION_ROTATE180:
                return desktop32[(capturedImageHeight - 1 - row) * desktopPitch + (capturedImageWidth - 1 - col)];
            case DXGI_MODE_ROTATION_ROTATE270:
                return desktop32[col * desktopPitch + (capturedImageHeight - 1 - row)];
            case DXGI_MODE_ROTATION_IDENTITY:
            case DXGI_MODE_ROTATION_UNSPECIFIED:
            default:
                return desktop32[row * desktopPitch + col];
        }
    };

    Buffer<BYTE> output;
    output.ExpandIfNeeded(bgraBuffer_.Size());

    // Access RGBA values at the same time
    auto output32 = output.As<UINT>();

    switch (GetType())
    {
        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
        {
            for (int row = rowMin, y = 0; row < rowMax; ++row, ++y)
            {
                for (int col = colMin, x = 0; col < colMax; ++col, ++x)
                {
                    const int i = col + row * cursorImageWidth;

                    BYTE mask = 0b10000000 >> (col % 8);
                    const BYTE andMask = apiBuffer_[col / 8 + row * cursorImagePitch] & mask;
                    const BYTE xorMask = apiBuffer_[col / 8 + (row + cursorImageHeight) * cursorImagePitch] & mask;
                    const UINT andMask32 = andMask ? 0xFFFFFFFF : 0x00000000;
                    const UINT xorMask32 = xorMask ? 0xFFFFFFFF : 0x00000000;

                    output32[i] = (getDesktop32(x, y) & andMask32) ^ xorMask32;
                }
            }
            break;
        }
        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
        {
            const auto buffer32 = apiBuffer_.As<UINT>();

            for (int row = rowMin, y = 0; row < rowMax; ++row, ++y)
            {
                for (int col = colMin, x = 0; col < colMax; ++col, ++x)
                {
                    const int i = col + row * cursorImageWidth;
                    const int j = col + row * cursorImagePitch / sizeof(UINT);

                    UINT mask = 0xFF000000 & buffer32[j];
                    if (mask)
                    {
                        output32[i] = (getDesktop32(x, y) ^ buffer32[j]) | 0xFF000000;
                    }
                    else
                    {
                        output32[i] = buffer32[j] | 0xFF000000;
                    }
                }
            }
            break;
        }
        case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
        {
            for (int row = rowMin, y = 0; row < rowMax; ++row, ++y)
            {
                for (int col = colMin, x = 0; col < colMax; ++col, ++x)
                {
                    const int i = 4 * (col + row * cursorImageWidth);

                    const auto desktop32 = getDesktop32(x, y);
                    const auto desktop = (BYTE*)(&desktop32);
                    const auto desktopB = desktop[0];
                    const auto desktopG = desktop[1];
                    const auto desktopR = desktop[2];
                    const auto desktopA = desktop[3];

                    const auto cursorB = apiBuffer_[i + 0];
                    const auto cursorG = apiBuffer_[i + 1];
                    const auto cursorR = apiBuffer_[i + 2];
                    const auto cursorA = apiBuffer_[i + 3];

                    const auto a0 = cursorA / 255.f;
                    const auto a1 = 1.f - a0;

                    output[i + 0] = static_cast<BYTE>(cursorB * a0 + desktopB * a1);
                    output[i + 1] = static_cast<BYTE>(cursorG * a0 + desktopG * a1);
                    output[i + 2] = static_cast<BYTE>(cursorR * a0 + desktopR * a1);
                    output[i + 3] = desktopA;
                }
            }
            break;
        }
        default:
        {
            Debug::Error("Cursor::UpdateTexture() => Unknown cursor type");
            break;
        }
    }

    // Rotation
    auto bgraBuffer32 = bgraBuffer_.As<UINT>();
    for (int row = rowMin, y = 0; row < rowMax; ++row, ++y)
    {
        for (int col = colMin, x = 0; col < colMax; ++col, ++x)
        {
            const auto i = col + row * cursorImageWidth;
            switch (monitorRot)
            {
                case DXGI_MODE_ROTATION_ROTATE90:
                    bgraBuffer32[i] = output32[col * cursorImageHeight + (cursorImageHeight - 1 - row)];
                    break;
                case DXGI_MODE_ROTATION_ROTATE180:
                    bgraBuffer32[i] = output32[(cursorImageHeight - 1 - row) * cursorImageWidth + (cursorImageWidth - 1 - col)];
                    break;
                case DXGI_MODE_ROTATION_ROTATE270:
                    bgraBuffer32[i] = output32[(cursorImageWidth - 1 - col) * cursorImageHeight + row];
                    break;
                case DXGI_MODE_ROTATION_IDENTITY:
                case DXGI_MODE_ROTATION_UNSPECIFIED:
                    bgraBuffer32[i] = output32[i];
                    break;
            }
        }
    }

    {
        ComPtr<ID3D11DeviceContext> context;
        GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(monitor->GetUnityTexture(), 0, &box, bgraBuffer_.Get(), GetWidth() * 4, 0);
    }

    if (FAILED(surface->Unmap()))
    {
        Debug::Error("Cursor::UpdateTexture() => surface->Unmap() failed.");
        return;
    }
}


void Cursor::GetTexture(ID3D11Texture2D* texture)
{
    if (!bgraBuffer_)
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

    ComPtr<ID3D11DeviceContext> context;
    GetDevice()->GetImmediateContext(&context);
    context->UpdateSubresource(texture, 0, nullptr, bgraBuffer_.Get(), GetWidth() * 4, 0);
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