#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include "Common.h"

class Duplicator;

class Cursor
{
public:
    explicit Cursor();
    ~Cursor();
    void UpdateBuffer(Duplicator* duplicator);
    void Draw(Duplicator* duplicator);
    void GetTexture(ID3D11Texture2D* texture);

    bool IsVisible() const;
    int GetX() const;
    int GetY() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetPitch() const;
    int GetType() const;

private:
    bool isVisible_ = false;
    int x_ = -1;
    int y_ = -1;
    Buffer<BYTE> buffer_;
    Buffer<BYTE> bgraBuffer_;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo_;
    LARGE_INTEGER timestamp_;
};