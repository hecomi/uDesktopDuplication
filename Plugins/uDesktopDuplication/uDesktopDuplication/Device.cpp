#pragma once

#include <queue>
#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Device.h"
#include "Debug.h"
#include "Common.h"

#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;



IsolatedD3D11Device::IsolatedD3D11Device()
{
}


IsolatedD3D11Device::~IsolatedD3D11Device()
{
}


HRESULT IsolatedD3D11Device::Create(const ComPtr<IDXGIAdapter>& adapter)
{
    UDD_FUNCTION_SCOPE_TIMER

    const auto driverType = adapter ? 
        D3D_DRIVER_TYPE_UNKNOWN : 
        D3D_DRIVER_TYPE_HARDWARE;
    const auto flags =
        D3D11_CREATE_DEVICE_BGRA_SUPPORT;       // D2D Compatible
        // | D3D11_CREATE_DEVICE_VIDEO_SUPPORT  // MediaFoundation
    const D3D_FEATURE_LEVEL featureLevelsRequested[] = 
    {
        D3D_FEATURE_LEVEL_11_0, 
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0, 
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2, 
        D3D_FEATURE_LEVEL_9_1 
    };
    const UINT numLevelsRequested = sizeof(featureLevelsRequested) / sizeof(D3D_FEATURE_LEVEL);
    D3D_FEATURE_LEVEL featureLevelsSupported;

    return D3D11CreateDevice(
        adapter.Get(),
        driverType,
        nullptr,
        flags,
        featureLevelsRequested,
        numLevelsRequested,
        D3D11_SDK_VERSION,
        &device_,
        &featureLevelsSupported,
        nullptr);
}


ComPtr<ID3D11Device> IsolatedD3D11Device::GetDevice()
{ 
    return device_;
}


Microsoft::WRL::ComPtr<ID3D11Texture2D> IsolatedD3D11Device::GetCompatibleSharedTexture(
    const ComPtr<ID3D11Texture2D>& src)
{
    UDD_FUNCTION_SCOPE_TIMER

    D3D11_TEXTURE2D_DESC srcDesc;
    src->GetDesc(&srcDesc);

    // check if the format and size of the current texture are same as the source one
    if (cachedSharedTexture_) 
    {
        D3D11_TEXTURE2D_DESC targetDesc;
        cachedSharedTexture_->GetDesc(&targetDesc);
        if (targetDesc.Format == srcDesc.Format && 
            targetDesc.Width  == srcDesc.Width  && 
            targetDesc.Height == srcDesc.Height)
        {
            return cachedSharedTexture_;
        }
    }

    // for sharing this texture with unity device
    srcDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    if (FAILED(device_->CreateTexture2D(&srcDesc, nullptr, &cachedSharedTexture_)))
    {
        Debug::Error("IsolatedD3D11Device::GetCompatibleSharedTexture() => Creating shared texture failed.");
        return nullptr;
    }

    return cachedSharedTexture_;
}