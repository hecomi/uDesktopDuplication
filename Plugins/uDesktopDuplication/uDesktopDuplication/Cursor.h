#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include "Common.h"

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
    Buffer<BYTE> apiBuffer_;
	Buffer<BYTE> bgra32Buffer_;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo_;
    LARGE_INTEGER timestamp_;
};