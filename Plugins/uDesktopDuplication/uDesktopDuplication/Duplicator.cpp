#pragma once

#include <chrono>

#include "Duplicator.h"
#include "Monitor.h"
#include "MonitorManager.h"
#include "Cursor.h"
#include "Device.h"
#include "Debug.h"

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"

using namespace Microsoft::WRL;



Duplicator::Duplicator(Monitor* monitor)
    : monitor_(monitor)
{
    InitializeDevice();
    InitializeDuplication();
    CheckUnityAdapter();
}


Duplicator::~Duplicator()
{
    Stop();
}


void Duplicator::InitializeDevice()
{
    const UINT cachedTextureNum = 2;
    device_ = std::make_shared<IsolatedD3D11Device>(cachedTextureNum);

    if (FAILED(device_->Create(monitor_->GetAdapter())))
    {
        Debug::Error("Monitor::Initialize() => IsolatedD3D11Device::Create() failed.");
        state_ = State::Unknown;
    }
}


void Duplicator::InitializeDuplication()
{
	ComPtr<IDXGIOutput1> output1;
	if (FAILED(monitor_->GetOutput().As(&output1))) {
		return;
	}

	auto hr = output1->DuplicateOutput(device_->GetDevice().Get(), &dupl_);
	switch (hr)
	{
		case S_OK:
		{
			state_ = State::Ready;
			const auto rot = static_cast<DXGI_MODE_ROTATION>(monitor_->GetRotation());
			Debug::Log("Duplicator::Initialize() => OK.");
			Debug::Log("    ID    : ", monitor_->GetId());
			Debug::Log("    Size  : (", monitor_->GetWidth(), ", ", monitor_->GetHeight(), ")");
			Debug::Log("    DPI   : (", monitor_->GetDpiX(), ", ", monitor_->GetDpiY(), ")");
			Debug::Log("    Rot   : ",
				rot == DXGI_MODE_ROTATION_IDENTITY ? "Landscape" :
				rot == DXGI_MODE_ROTATION_ROTATE90 ? "Portrait" :
				rot == DXGI_MODE_ROTATION_ROTATE180 ? "Landscape (flipped)" :
				rot == DXGI_MODE_ROTATION_ROTATE270 ? "Portrait (flipped)" :
				"Unspecified");
			break;
		}
		case E_INVALIDARG:
		{
			state_ = State::InvalidArg;
			Debug::Error("Duplicator::Initialize() => Invalid arguments.");
			break;
		}
		case E_ACCESSDENIED:
		{
			// For example, when the user presses Ctrl + Alt + Delete and the screen
			// switches to admin screen, this error occurs. 
			state_ = State::AccessDenied;
			Debug::Error("Duplicator::Initialize() => Access denied.");
			break;
		}
		case DXGI_ERROR_UNSUPPORTED:
		{
			// If the display adapter on the computer is running under the Microsoft Hybrid system,
			// this error occurs.
			state_ = State::Unsupported;
			Debug::Error("Duplicator::Initialize() => Unsupported display.");
			break;
		}
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
		{
			// When other application use Desktop Duplication API, this error occurs.
			state_ = State::CurrentlyNotAvailable;
			Debug::Error("Duplicator::Initialize() => Currently not available.");
			break;
		}
		case DXGI_ERROR_SESSION_DISCONNECTED:
		{
			state_ = State::SessionDisconnected;
			Debug::Error("Duplicator::Initialize() => Session disconnected.");
			break;
		}
		default:
		{
			state_ = State::Unknown;
			Debug::Error("Duplicator::Render() => Unknown Error.");
			break;
		}
	}
}


void Duplicator::CheckUnityAdapter()
{
    DXGI_ADAPTER_DESC desc;
    monitor_->GetAdapter()->GetDesc(&desc);

    const auto unityAdapterLuid = GetUnityAdapterLuid();
    const auto isUnityAdapter =
        (desc.AdapterLuid.LowPart  == unityAdapterLuid.LowPart) &&
        (desc.AdapterLuid.HighPart == unityAdapterLuid.HighPart);

    if (!isUnityAdapter)
    {
        Debug::Error("Duplicator::CheckUnityAdapter() => The adapter is not same as Unity, and now this case is not supported.");
        state_ = State::Unsupported;
    }
}


void Duplicator::Start()
{
    if (state_ != State::Ready) return;

    Stop();

    thread_ = std::thread([this] 
    {
        using namespace std::chrono;

        state_ = State::Running;

        shouldRun_ = true;
        while (shouldRun_)
        {
            const auto frameRate = GetMonitorManager()->GetFrameRate();
            const UINT frameMicroSeconds = 1000000 / frameRate;
            const UINT frameMilliSeconds = 1000 / frameRate;

            ScopedTimer timer([frameMicroSeconds] (microseconds us)
            {
                const auto waitTime = microseconds(frameMicroSeconds) - us;
                if (waitTime > microseconds::zero())
                {
                    std::this_thread::sleep_for(waitTime);
                }
            });

            const auto timeout = static_cast<UINT>(frameMilliSeconds);
            Duplicate(timeout);

            if (state_ != State::Running)
            {
                break;
            }
        }

        if (state_ == State::Running)
        {
            state_ = State::Ready;
        }
    });
}


void Duplicator::Stop()
{
    shouldRun_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}


bool Duplicator::IsRunning() const
{
    return state_ == State::Running;
}


bool Duplicator::IsError() const
{
    return
        state_ != State::Ready &&
        state_ != State::Running;
}


Duplicator::State Duplicator::GetState() const
{
    return state_;
}


Monitor* Duplicator::GetMonitor() const
{
    return monitor_;
}


Microsoft::WRL::ComPtr<ID3D11Device> Duplicator::GetDevice()
{
    return device_->GetDevice();
}


ComPtr<IDXGIOutputDuplication> Duplicator::GetDuplication()
{
    return dupl_;
}


const Duplicator::Frame& Duplicator::GetLastFrame() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return lastFrame_;
}


void Duplicator::Duplicate(UINT timeout)
{
    if (!dupl_ || !device_) return;

    ComPtr<IDXGIResource> resource;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    const auto hr = dupl_->AcquireNextFrame(timeout, &frameInfo, &resource);

    if (FAILED(hr)) 
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                // If any monitor setting has changed (e.g. monitor size has changed),
                // it is necessary to re-initialize monitors.
                Debug::Log("Duplicator::Duplicate() => DXGI_ERROR_ACCESS_LOST.");
                state_ = State::AccessLost;
                break;
            }
            case DXGI_ERROR_WAIT_TIMEOUT:
            {
                // This often occurs when timeout value is small and it is not problem. 
                // Debug::Log("Duplicator::Duplicate() => DXGI_ERROR_WAIT_TIMEOUT.");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Duplicator::Duplicate() => DXGI_ERROR_INVALID_CALL.");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Duplicator::Duplicate() => E_INVALIDARG.");
                break;
            }
            default:
            {
                state_ = State::Unknown;
                Debug::Error("Duplicator::Duplicate() => Unknown Error.");
                break;
            }
        }
        return;
    }

    ScopedReleaser releaser([this] 
    {
        const auto hr = dupl_->ReleaseFrame();
        if (FAILED(hr))
        {
            switch (hr)
            {
                case DXGI_ERROR_ACCESS_LOST:
                {
                    Debug::Log("Duplicator::Duplicate() => DXGI_ERROR_ACCESS_LOST.");
                    state_ = State::AccessLost;
                    break;
                }
                case DXGI_ERROR_INVALID_CALL:
                {
                    Debug::Error("Duplicator::Duplicate() => DXGI_ERROR_INVALID_CALL.");
                    break;
                }
                default:
                {
                    state_ = State::Unknown;
                    Debug::Error("Duplicator::Duplicate() => Unknown Error.");
                    break;
                }
            }
        }
    });

    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource.As(&texture))) 
    {
        Debug::Error("Duplicator::Duplicate() => IDXGIResource could not be converted to ID3D11Texture2D.");
        return;
    }

    //auto sharedTextureWrapper = device_->GetCompatibleSharedTexture(texture, lastFrameId_ % 2);
    auto sharedTextureWrapper = device_->GetCompatibleSharedTexture(texture, 0);
    if (!sharedTextureWrapper)
    {
        Debug::Error("Duplicator::Duplicate() => Shared texture is null.");
        return;
    }

    auto sharedTexture = sharedTextureWrapper->Lock();
    if (!sharedTexture) return;
    ScopedReleaser sharedTextureReleaser([&] { sharedTextureWrapper->Unlock(); });

    ComPtr<ID3D11DeviceContext> context;
    device_->GetDevice()->GetImmediateContext(&context);
    context->CopyResource(sharedTexture.Get(), texture.Get());

    UpdateCursor(sharedTexture, frameInfo);
    UpdateMetadata(frameInfo.TotalMetadataBufferSize);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastFrame_ = Frame
        {
            lastFrameId_++,
            sharedTextureWrapper,
            frameInfo,
            metaData_
        };
    }
}


void Duplicator::UpdateCursor(
    const ComPtr<ID3D11Texture2D>& texture,
    const DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
    auto& manager = GetMonitorManager();

	if (frameInfo.PointerPosition.Visible)
	{
        manager->SetCursorMonitorId(monitor_->GetId());
	}

    if (monitor_->GetId() == manager->GetCursorMonitorId())
    {
        auto cursor = manager->GetCursor();
        cursor->UpdateBuffer(this, frameInfo);
        cursor->Draw(this, texture);
    }
}


void Duplicator::UpdateMetadata(UINT totalBufferSize)
{
    metaData_.buffer.ExpandIfNeeded(totalBufferSize);
    if (!metaData_.buffer.Empty())
    {
        UpdateMoveRects();
        UpdateDirtyRects();
    }
}


void Duplicator::UpdateMoveRects()
{
    const auto hr = dupl_->GetFrameMoveRects(
        metaData_.buffer.Size(),
        metaData_.buffer.As<DXGI_OUTDUPL_MOVE_RECT>(), 
        &metaData_.moveRectSize);

    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                Debug::Log("Duplicator::UpdateMoveRects() => DXGI_ERROR_ACCESS_LOST.");
                break;
            }
            case DXGI_ERROR_MORE_DATA:
            {
                Debug::Error("Duplicator::UpdateMoveRects() => DXGI_ERROR_MORE_DATA.");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Duplicator::UpdateMoveRects() => DXGI_ERROR_INVALID_CALL.");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Duplicator::UpdateMoveRects() => E_INVALIDARG.");
                break;
            }
            default:
            {
                Debug::Error("Duplicator::UpdateMoveRects() => Unknown Error.");
                break;
            }
        }
        return;
    }
}


void Duplicator::UpdateDirtyRects()
{
    const auto hr = dupl_->GetFrameDirtyRects(
        metaData_.buffer.Size() - metaData_.moveRectSize,
        metaData_.buffer.As<RECT>(metaData_.moveRectSize /* offset */), 
        &metaData_.dirtyRectSize);

    if (FAILED(hr))
    {
        switch (hr)
        {
            case DXGI_ERROR_ACCESS_LOST:
            {
                Debug::Log("Duplicator::UpdateDirtyRects() => DXGI_ERROR_ACCESS_LOST.");
                break;
            }
            case DXGI_ERROR_MORE_DATA:
            {
                Debug::Error("Duplicator::UpdateDirtyRects() => DXGI_ERROR_MORE_DATA.");
                break;
            }
            case DXGI_ERROR_INVALID_CALL:
            {
                Debug::Error("Duplicator::UpdateDirtyRects() => DXGI_ERROR_INVALID_CALL.");
                break;
            }
            case E_INVALIDARG:
            {
                Debug::Error("Duplicator::UpdateDirtyRects() => E_INVALIDARG.");
                break;
            }
            default:
            {
                Debug::Error("Duplicator::UpdateDirtyRects() => Unknown Error.");
                break;
            }
        }
        return;
    }
}