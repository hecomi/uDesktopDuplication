#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>

class Monitor;

class Cursor
{
public:
    explicit Cursor(Monitor* monitor);
    ~Cursor();
    void UpdateBuffer(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
    void UpdateTexture();
    void GetTexture(ID3D11Texture2D* texture);

    bool IsVisible() const;
    int GetX() const;
    int GetY() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetPitch() const;
    int GetType() const;

private:
    bool IsCursorOnParentMonitor() const;

    Monitor* monitor_;
    bool isVisible_ = false;
    int x_ = -1;
    int y_ = -1;
    std::unique_ptr<BYTE[]> apiBuffer_ = nullptr;
    UINT apiBufferSize_ = 0;
    std::unique_ptr<BYTE[]> bgra32Buffer_ = nullptr;
    UINT bgra32BufferSize_ = 0;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo_;
    LARGE_INTEGER timestamp_;
};