#include <d3d11.h>

#include "Cursor.h"
#include "Debug.h"
#include "Monitor.h"
#include "Duplicator.h"

using namespace Microsoft::WRL;



Cursor::Cursor()
{
}


Cursor::~Cursor()
{
}


void Cursor::UpdateBuffer(Duplicator* duplicator, const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    if (frameInfo.LastMouseUpdateTime.QuadPart == 0)
    {
        return;
    }

    isVisible_ = frameInfo.PointerPosition.Visible != 0;
    x_ = frameInfo.PointerPosition.Position.x;
    y_ = frameInfo.PointerPosition.Position.y;
    timestamp_ = frameInfo.LastMouseUpdateTime;

    if (frameInfo.PointerShapeBufferSize == 0)
    {
        return;
    }

    buffer_.ExpandIfNeeded(frameInfo.PointerShapeBufferSize);
    if (!buffer_) 
    {
        return;
    }

    // Get mouse pointer information
    UINT bufferSize;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;
    const auto hr = duplicator->GetDuplication()->GetFramePointerShape(
        buffer_.Size(),
        buffer_.Get(),
        &bufferSize,
        &shapeInfo);

    if (FAILED(hr))
    {
        Debug::Error("Cursor::UpdateBuffer() => GetFramePointerShape() failed.");
        buffer_.Reset();
        return;
    }

    shapeInfo_ = shapeInfo;
}


void Cursor::UpdateTexture(
    Duplicator* duplicator, 
    const ComPtr<ID3D11Texture2D>& desktopTexture)
{
    auto monitor = duplicator->GetMonitor();

    // Check desktop texure
    if (desktopTexture == nullptr) 
    {
        Debug::Error("Cursor::UpdateTexture() => Desktop texture is null.");
        return;
    }

    // Cursor information
    const auto cursorImageWidth  = GetWidth();
    const auto cursorImageHeight = GetHeight();
    const auto cursorImagePitch  = GetPitch();

    // Monitor orientation
    const auto monitorRot = static_cast<DXGI_MODE_ROTATION>(monitor->GetRotation());
    const auto isMonitorPortrait = 
        monitorRot == DXGI_MODE_ROTATION_ROTATE90 || 
        monitorRot == DXGI_MODE_ROTATION_ROTATE270;

    // Captured size (desktop cooridinates).
    auto capturedImageWidth  = !isMonitorPortrait ? cursorImageWidth  : cursorImageHeight;
    auto capturedImageHeight = !isMonitorPortrait ? cursorImageHeight : cursorImageWidth;

    // Convert the buffer given by API into BGRA32
    const UINT bgraBufferSize = cursorImageWidth * cursorImageHeight * 4;
    bgraBuffer_.ExpandIfNeeded(bgraBufferSize);
    
    // Check buffers
    if (!bgraBuffer_ || !buffer_) 
    {
        Debug::Error("Cursor::UpdateTexture() => no buffer.");
        return;
    }

    // Desktop size
    const int monitorWidth = monitor->GetWidth();
    const int monitorHeight = monitor->GetHeight();
    const int desktopImageWidth  = !isMonitorPortrait ? monitorWidth  : monitorHeight;
    const int desktopImageHeight = !isMonitorPortrait ? monitorHeight : monitorWidth;

    // x_, y_ are cooridinates in rotated monitor.
    // desktopX, desktopY are coordinates in captured desktop image (always landscape).
    int desktopX, desktopY;
    switch (monitorRot)
    {
        case DXGI_MODE_ROTATION_ROTATE90:
        {
            desktopX = y_;
            desktopY = (desktopImageHeight - 1) - x_ - cursorImageWidth;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE180:
        {
            desktopX = (desktopImageWidth  - 1) - x_ - cursorImageWidth;
            desktopY = (desktopImageHeight - 1) - y_ - cursorImageHeight;
            break;
        }
        case DXGI_MODE_ROTATION_ROTATE270:
        {
            desktopX = (desktopImageWidth - 1) - y_ - cursorImageHeight;
            desktopY = x_;
            break;
        }
        case DXGI_MODE_ROTATION_IDENTITY:
        case DXGI_MODE_ROTATION_UNSPECIFIED:
        default:
        {
            desktopX = x_;
            desktopY = y_;
            break;
        }
    }

    // Calculate information to capture desktop image under cursor.
    int cursorOffsetX = 0;
    int cursorOffsetY = 0;
    int capturedImageLeft   = desktopX;
    int capturedImageTop    = desktopY;
    int capturedImageRight  = desktopX + capturedImageWidth;
    int capturedImageBottom = desktopY + capturedImageHeight;

    if (capturedImageLeft < 0)
    {
        capturedImageWidth -= -desktopX;
        cursorOffsetX = -desktopX;
        capturedImageLeft = 0;
    }

    if (capturedImageRight >= desktopImageWidth) 
    {
        capturedImageWidth -= capturedImageRight - desktopImageWidth;
        capturedImageRight = desktopImageWidth - 1;
    }

    if (capturedImageTop < 0)
    {
        capturedImageHeight -= -desktopY;
        cursorOffsetY = -desktopY;
        capturedImageTop = 0;
    }

    if (capturedImageBottom >= desktopImageHeight) 
    {
        capturedImageHeight -= capturedImageBottom - desktopImageHeight;
        capturedImageBottom = desktopImageHeight - 1;
    }

    // Check if box is inner desktop area
    if (capturedImageLeft   < 0 || 
        capturedImageTop    < 0 || 
        capturedImageRight  >= desktopImageWidth || 
        capturedImageBottom >= desktopImageHeight)
    {
        Debug::Error("Cursor::UpdateTexture() => box is out of area.");
        Debug::Error(
            "    ",
            "(", capturedImageLeft, ", ", capturedImageTop, ")", 
            " ~ (", capturedImageRight, ", ", capturedImageBottom, ") > ",
            "(", desktopImageWidth, ", ", desktopImageHeight, ")");
        return;
    }

    if (capturedImageWidth == 0 || capturedImageHeight == 0)
    {
        return;
    }

    capturedImageArea_ = D3D11_BOX 
    { 
        static_cast<UINT>(capturedImageLeft), 
        static_cast<UINT>(capturedImageTop),
        0, 
        static_cast<UINT>(capturedImageRight), 
        static_cast<UINT>(capturedImageBottom), 
        1 
    };

    // Create texture for capturing desktop image
    ComPtr<ID3D11Texture2D> texture;
    {
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = capturedImageWidth;
        desc.Height             = capturedImageHeight;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_STAGING;
        desc.BindFlags          = 0;
        desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags          = D3D11_RESOURCE_MISC_SHARED;

        if (FAILED(duplicator->GetDevice()->CreateTexture2D(&desc, nullptr, &texture)))
        {
            Debug::Error("Cursor::UpdateTexture() => GetDevice()->CreateTexture2D() failed.");
            return;
        }
    }

    // Copy desktop image to the texture
    {
        ComPtr<ID3D11DeviceContext> context;
        duplicator->GetDevice()->GetImmediateContext(&context);
        context->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0, desktopTexture.Get(), 0, &capturedImageArea_);
    }

    // Get mapped surface to access pixels in CPU
    ComPtr<IDXGISurface> surface;
    if (FAILED(texture.As(&surface)))
    {
        Debug::Error("Cursor::UpdateTexture() => texture.As() failed.");
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

    // Rotate cursor image to match the monitor orientation
    Buffer<BYTE> rotatedBuffer_;
    rotatedBuffer_.ExpandIfNeeded(buffer_.Size());

    for (int y = 0; y < capturedImageHeight; ++y)
    {
        for (int x = 0; x < capturedImageWidth; ++x)
        {
            // Cursor coordinates
            int cursorX, cursorY;

            switch (monitorRot)
            {
                case DXGI_MODE_ROTATION_ROTATE90:
                {
                    cursorX = (cursorImageWidth - 1) - (y + cursorOffsetY);
                    cursorY = (x + cursorOffsetX);
                    break;
                }
                case DXGI_MODE_ROTATION_ROTATE180:
                {
                    cursorX = (cursorImageWidth  - 1) - (x + cursorOffsetX);
                    cursorY = (cursorImageHeight - 1) - (y + cursorOffsetY);
                    break;
                }
                case DXGI_MODE_ROTATION_ROTATE270:
                {
                    cursorX = (y + cursorOffsetY);
                    cursorY = (cursorImageHeight - 1) - (x + cursorOffsetX);
                    break;
                }
                case DXGI_MODE_ROTATION_IDENTITY:
                case DXGI_MODE_ROTATION_UNSPECIFIED:
                default:
                {
                    cursorX = (x + cursorOffsetX);
                    cursorY = (y + cursorOffsetY);
                    break;
                }
            }

            const auto outputIndex = y * capturedImageWidth + x;
            const auto desktopIndex = y * desktopPitch + x;
            const auto cursorIndex = cursorY * cursorImageWidth + cursorX;
            const auto buffer32 = buffer_.As<UINT>();
            auto output32 = bgraBuffer_.As<UINT>();

            switch (GetType())
            {
                case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
                {
                    BYTE mask = 0b10000000 >> (cursorX % 8);
                    const BYTE andMask = buffer_[cursorX / 8 + cursorY * cursorImagePitch] & mask;
                    const BYTE xorMask = buffer_[cursorX / 8 + (cursorY + cursorImageHeight) * cursorImagePitch] & mask;
                    const UINT andMask32 = andMask ? 0xFFFFFFFF : 0x00000000;
                    const UINT xorMask32 = xorMask ? 0xFFFFFFFF : 0x00000000;
                    output32[outputIndex] = (desktop32[desktopIndex] & andMask32) ^ xorMask32;
                    break;
                }
                case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
                {
                    UINT mask = 0xFF000000 & buffer32[cursorIndex];
                    if (mask)
                    {
                        output32[outputIndex] = (desktop32[desktopIndex] ^ buffer32[cursorIndex]) | 0xFF000000;
                    }
                    else
                    {
                        output32[outputIndex] = buffer32[cursorIndex] | 0xFF000000;
                    }
                    break;
                }
                case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
                {
                    const auto desktop = reinterpret_cast<BYTE*>(&desktop32[desktopIndex]);
                    const auto desktopB = desktop[0];
                    const auto desktopG = desktop[1];
                    const auto desktopR = desktop[2];
                    const auto desktopA = desktop[3];

                    const auto cursor = buffer_.Get(cursorIndex * 4);
                    const auto cursorB = cursor[0];
                    const auto cursorG = cursor[1];
                    const auto cursorR = cursor[2];
                    const auto cursorA = cursor[3];

                    const auto a0 = cursorA / 255.f;
                    const auto a1 = 1.f - a0;

                    auto output = reinterpret_cast<BYTE*>(&output32[outputIndex]);
                    output[0] = static_cast<BYTE>(cursorB * a0 + desktopB * a1);
                    output[1] = static_cast<BYTE>(cursorG * a0 + desktopG * a1);
                    output[2] = static_cast<BYTE>(cursorR * a0 + desktopR * a1);
                    output[3] = desktopA;

                    break;
                }
                default:
                {
                    Debug::Error("Cursor::UpdateTexture() => Unknown cursor type");
                    return;
                }
            }
        }
    }

    if (FAILED(surface->Unmap()))
    {
        Debug::Error("Cursor::UpdateTexture() => surface->Unmap() failed.");
        return;
    }
}


void Cursor::Draw(const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture)
{
    if (texture == nullptr) 
    {
        Debug::Error("Cursor::UpdateTexture() => Desktop texture is null.");
        return;
    }

    const auto capturedImageWidth = capturedImageArea_.right - capturedImageArea_.left;
    ComPtr<ID3D11DeviceContext> context;
    GetDevice()->GetImmediateContext(&context);
    context->UpdateSubresource(texture.Get(), 0, &capturedImageArea_, bgraBuffer_.Get(), capturedImageWidth * 4, 0);
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