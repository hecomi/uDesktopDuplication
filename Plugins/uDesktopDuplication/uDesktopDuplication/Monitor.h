#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include <mutex>
#include <thread>
#include "Common.h"


enum class DuplicatorState;


class Monitor
{
	class ThreadedDesktopDuplicator *m_threaded = nullptr;

public:
    explicit Monitor(int id);
    ~Monitor();
	void Initialize(
        const Microsoft::WRL::ComPtr<struct IDXGIAdapter> &adapter,
		const Microsoft::WRL::ComPtr<struct IDXGIOutput> &output);
    void Finalize();
    void Render();

public:
    int GetId() const;
    Microsoft::WRL::ComPtr<struct IDXGIAdapter> GetAdapter();
    Microsoft::WRL::ComPtr<struct IDXGIOutput> GetOutput();
    DuplicatorState GetDuplicatorState() const;
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
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> GetDeskDupl();
    int GetMoveRectCount() const;
    DXGI_OUTDUPL_MOVE_RECT* GetMoveRects() const;
    int GetDirtyRectCount() const;
    RECT* GetDirtyRects() const;
    void UseGetPixels(bool use);
    bool UseGetPixels() const;
    bool GetPixels(BYTE* output, int x, int y, int width, int height);

private:
    void CopyTextureFromGpuToCpu(ID3D11Texture2D* texture);

    int id_ = -1;

    UINT dpiX_ = -1, dpiY_ = -1;
    int width_ = -1, height_ = -1;

    bool hasBeenUpdated_ = false;
    bool useGetPixels_ = false;

    Microsoft::WRL::ComPtr<IDXGIOutput> output_;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter_;
    DXGI_OUTPUT_DESC outputDesc_;
    MONITORINFOEX monitorInfo_;

    std::shared_ptr<class Duplicator> duplicator_;

    ID3D11Texture2D* unityTexture_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> textureForGetPixels_;
    Buffer<BYTE> bufferForGetPixels_;
};
