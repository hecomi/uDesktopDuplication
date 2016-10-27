#include <d3d11.h>
#include <dxgi1_2.h>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"

#pragma comment(lib, "dxgi.lib")


namespace
{
    IUnityInterfaces*       g_unity            = nullptr;
    IDXGIOutputDuplication* g_deskDupl         = nullptr;
    ID3D11Texture2D*        g_texture          = nullptr;
    bool                    g_isPointerVisible = -1;
    int                     g_pointerX         = -1;
    int                     g_pointerY         = -1;
    int                     g_width            = -1;
    int                     g_height           = -1;
}


extern "C"
{
    void InitializeDuplication()
    {
        IDXGIFactory1* factory;
        CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));

        // Check all display adapters.
        IDXGIAdapter1* adapter;
        for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
        {
            // Search the main monitor from all outputs.
            IDXGIOutput* output;
            for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); j++) 
            {
                DXGI_OUTPUT_DESC outputDesc;
                output->GetDesc(&outputDesc);

                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                GetMonitorInfo(outputDesc.Monitor, &monitorInfo);

                if (monitorInfo.dwFlags == MONITORINFOF_PRIMARY) 
                {
                    g_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
                    g_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

                    auto device = g_unity->Get<IUnityGraphicsD3D11>()->GetDevice();
                    IDXGIOutput1* output1;
                    output1 = reinterpret_cast<IDXGIOutput1*>(output);
                    output1->DuplicateOutput(device, &g_deskDupl);

                    output->Release();
                    adapter->Release();
                    factory->Release();

                    return;
                }

                output->Release();
            }

            adapter->Release();
        }

        factory->Release();
    }

    void FinalizeDuplication()
    {
        g_deskDupl->Release();
        g_deskDupl = nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
        InitializeDuplication();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        g_unity = nullptr;
        g_texture = nullptr;
        g_width = -1;
        g_height = -1;
        g_pointerX = -1;
        g_pointerY = -1;
        FinalizeDuplication();
    }

    void UNITY_INTERFACE_API OnRenderEvent(int eventId)
    {
        if (g_deskDupl == nullptr || g_texture == nullptr) return;

        HRESULT hr;
        IDXGIResource* resource = nullptr;
        DXGI_OUTDUPL_FRAME_INFO frameInfo;

        const UINT timeout = 100; // ms
        hr = g_deskDupl->AcquireNextFrame(timeout, &frameInfo, &resource);
        switch (hr) 
        {
            case S_OK: 
            {
                break;
            }
            case DXGI_ERROR_ACCESS_LOST: 
            {
                FinalizeDuplication();
                InitializeDuplication();
                return;
            }
            default: 
            {
                return;
            }
        }

        g_isPointerVisible = frameInfo.PointerPosition.Visible;
        g_pointerX = frameInfo.PointerPosition.Position.x;
        g_pointerY = frameInfo.PointerPosition.Position.y;

        ID3D11Texture2D* texture;
        hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
        if (hr != S_OK) 
        {
            resource->Release();
            return;
        }

        resource->Release();

        ID3D11DeviceContext* context;
        auto device = g_unity->Get<IUnityGraphicsD3D11>()->GetDevice();
        device->GetImmediateContext(&context);
        context->CopyResource(g_texture, texture);

        g_deskDupl->ReleaseFrame();
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth()
    {
        return g_width;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight()
    {
        return g_height;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API IsPointerVisible()
    {
        return g_isPointerVisible;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerX()
    {
        return g_pointerX;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerY()
    {
        return g_pointerY;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(void* texture)
    {
        g_texture = reinterpret_cast<ID3D11Texture2D*>(texture);
    }
}