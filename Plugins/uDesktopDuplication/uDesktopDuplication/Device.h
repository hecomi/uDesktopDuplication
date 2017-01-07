#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>


// Shared texture wrapper to manager locking state
class SharedTextureWrapper
{
friend class IsolatedD3D11Device;
public:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Get();
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Lock();
    void Unlock();
    bool IsLocked() const;

private:
    std::atomic<bool> locked_ = false;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pointer_;
};


// Thraed safe self created ID3D11Device from specified adapter
class IsolatedD3D11Device
{
public:
    explicit IsolatedD3D11Device(UINT cachedTextureNum);
    ~IsolatedD3D11Device();

    HRESULT Create(const Microsoft::WRL::ComPtr<IDXGIAdapter>& adapter);
    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
    std::shared_ptr<SharedTextureWrapper> GetCompatibleSharedTexture(
        const Microsoft::WRL::ComPtr<ID3D11Texture2D>& src,
        UINT index);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    std::vector<std::shared_ptr<SharedTextureWrapper>> cachedSharedTextures_;
};
