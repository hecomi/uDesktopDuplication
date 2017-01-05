#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <wrl/client.h>

#include "Common.h"


class Monitor;


enum class DuplicatorState
{
    NotSet = -1,
    Ready = 0,
    Running = 1,
    InvalidArg = 2,
    AccessDenied = 3,
    Unsupported = 4,
    CurrentlyNotAvailable = 5,
    SessionDisconnected = 6,
    AccessLost = 7,
    TextureSizeInconsistent = 8,
    Unknown = 999,
};


class Duplicator
{
public:
    using State = DuplicatorState;

    struct Metadata
    {
        Buffer<BYTE> buffer;
        UINT moveRectSize = 0;
        UINT dirtyRectSize = 0;
    };

    struct Frame
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        DXGI_OUTDUPL_FRAME_INFO info;
        Metadata metaData;
    };

    explicit Duplicator(Monitor* monitor);
    ~Duplicator();
    void Start();
    void Stop();
    bool IsRunning() const;
    bool IsError() const;
    State GetState() const;
    Monitor* GetMonitor() const;
    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> GetDuplication();
    const Frame& GetFrame() const;

private:
    void InitializeDevice();
    void InitializeDuplication();
    void CheckUnityAdapter();

    bool Duplicate();

    void UpdateCursor();
	void UpdateMetadata();
    void UpdateMoveRects();
    void UpdateDirtyRects();

    Monitor* monitor_ = nullptr;
    std::atomic<State> state_ = State::Ready;

    std::shared_ptr<class IsolatedD3D11Device> device_;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dupl_;
    Frame lastFrame_;

    volatile bool shouldRun_ = false;
    std::thread thread_;
    mutable std::mutex mutex_;

    Metadata metaData_;
};
