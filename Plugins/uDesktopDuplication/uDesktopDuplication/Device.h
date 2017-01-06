#pragma once

#include <vector>
#include <d3d11.h>
#include <wrl/client.h>


// Thraed safe self created ID3D11Device from specified adapter
class IsolatedD3D11Device
{
public:
    explicit IsolatedD3D11Device(UINT cachedTextureNum);
    ~IsolatedD3D11Device();

    HRESULT Create(const Microsoft::WRL::ComPtr<IDXGIAdapter>& adapter);
    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
    Microsoft::WRL::ComPtr<ID3D11Texture2D> GetCompatibleSharedTexture(
        const Microsoft::WRL::ComPtr<ID3D11Texture2D>& src,
        UINT index);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> cachedTextures_;
};
