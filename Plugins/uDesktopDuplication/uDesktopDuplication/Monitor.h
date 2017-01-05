#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include <mutex>
#include <thread>
#include "Common.h"

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
	class ThreadedDesktopDuplicator *m_threaded = nullptr;

public:
    using State = MonitorState;

    explicit Monitor(int id);
    ~Monitor();
	void Initialize(
        const Microsoft::WRL::ComPtr<struct IDXGIAdapter> &adapter,
		const Microsoft::WRL::ComPtr<struct IDXGIOutput> &output);
	void CopyTextureFromThread();
    void Render(UINT timeout = 0);

public:
    int GetId() const;
    State GetState() const;
    void SetUnityTexture(ID3D11Texture2D* texture);
    ID3D11Texture2D* GetUnityTexture() const;
    void GetName(char* buf, int len) const;
    bool IsPrimary() const;
    bool HasBeenUpdated() const;
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
    int GetMoveRectCount() const;
    DXGI_OUTDUPL_MOVE_RECT* GetMoveRects() const;
    int GetDirtyRectCount() const;
    RECT* GetDirtyRects() const;
    void UseGetPixels(bool use);
    bool UseGetPixels() const;
    bool GetPixels(BYTE* output, int x, int y, int width, int height);

private:
	void DuplicateAndCopyLoop();
	void DuplicateAndMapLoop();
	std::shared_ptr<class IsolatedD3D11Device> m_pIsolated;
	std::thread m_desktopDuplicationThread;
	volatile bool m_stopLoop = false;
	std::shared_ptr<class TextureQueue> m_textureQueue;

private:
    void UpdateCursor(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
    void UpdateMetadata(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
    void UpdateMoveRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
    void UpdateDirtyRects(const DXGI_OUTDUPL_FRAME_INFO& frameInfo);
    void CopyTextureFromGpuToCpu(ID3D11Texture2D* texture);

    int id_ = -1;
    UINT dpiX_ = -1, dpiY_ = -1;
    int width_ = -1, height_ = -1;
    bool hasBeenUpdated_ = false;
    bool useGetPixels_ = false;
    State state_ = State::NotSet;
    IDXGIOutputDuplication* deskDupl_ = nullptr;
    ID3D11Texture2D* unityTexture_ = nullptr;
    DXGI_OUTPUT_DESC outputDesc_;
    MONITORINFOEX monitorInfo_;
    Buffer<BYTE> metaData_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> textureForGetPixels_;
    Buffer<BYTE> bufferForGetPixels_;
    UINT moveRectSize_ = 0;
    UINT dirtyRectSize_ = 0;;
};
