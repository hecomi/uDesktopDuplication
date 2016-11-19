#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include "Common.h"

class Cursor;

enum class MonitorState
{
    NotSet = -1,
    Available = 0,
    InvalidArg = 1,
    AccessDenied = 2,
    Unsupported = 3,
    CurrentlyNotAvailable = 4,
    SessionDisconnected = 5,
    AccessLost = 6,
    TextureSizeInconsistent = 7,
    Unknown = 999,
};

class Monitor
{
public:
    using State = MonitorState;

    explicit Monitor(int id);
    ~Monitor();
    void Initialize(IDXGIOutput* output);
    void Render(UINT timeout = 0);
    void GetCursorTexture(ID3D11Texture2D* texture);

public:
    int GetId() const;
    State GetState() const;
    void SetUnityTexture(ID3D11Texture2D* texture);
    ID3D11Texture2D* GetUnityTexture() const;
    void GetName(char* buf, int len) const;
    bool IsPrimary() const;
    int GetLeft() const;
    int GetRight() const;
    int GetTop() const;
    int GetBottom() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetRotation() const;
    int GetDpiX() const;
    int GetDpiY() const;
    IDXGIOutputDuplication* GetDeskDupl();
    const std::unique_ptr<Cursor>& GetCursor();
	int GetMoveRectCount() const;
	DXGI_OUTDUPL_MOVE_RECT* GetMoveRects() const;
	int GetDirtyRectCount() const;
	RECT* GetDirtyRects() const;

private:
	void UpdateCursor(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
	void UpdateMetadata(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
	void UpdateMoveRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
	void UpdateDirtyRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);

    int id_ = -1;
    UINT dpiX_ = -1, dpiY_ = -1;
    int width_ = -1, height_ = -1;
    State state_ = State::NotSet;
    std::unique_ptr<Cursor> cursor_;
    IDXGIOutputDuplication* deskDupl_ = nullptr;
    ID3D11Texture2D* unityTexture_ = nullptr;
    DXGI_OUTPUT_DESC outputDesc_;
    MONITORINFOEX monitorInfo_;
	Buffer<BYTE> metaData_;
	UINT moveRectSize_ = 0;
	UINT dirtyRectSize_ = 0;;
};