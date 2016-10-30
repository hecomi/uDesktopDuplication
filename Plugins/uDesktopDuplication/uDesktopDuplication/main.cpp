#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"

#pragma comment(lib, "dxgi.lib")


namespace
{
    struct Monitor
    {
        IDXGIOutputDuplication* output           = nullptr;
        ID3D11Texture2D*        texture          = nullptr;
        std::string             name             = "";
        bool                    isPrimary        = false;
        bool                    isPointerVisible = false;
        int                     pointerX         = -1;
        int                     pointerY         = -1;
        int                     width            = -1;
        int                     height           = -1;
    };

    IUnityInterfaces* g_unity = nullptr;
    int g_timeout = 10;
    HRESULT g_errorCode = 0;
    std::vector<Monitor> g_monitors;
}


extern "C"
{
    void FinalizeDuplication()
    {
        for (auto& monitor : g_monitors)
        {
            monitor.output->Release();
        }
        g_monitors.clear();
    }

    void InitializeDuplication()
    {
        FinalizeDuplication();

        IDXGIFactory1* factory;
        CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));

        // Check all display adapters.
        IDXGIAdapter1* adapter;
        for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
        {
            // Search the main monitor from all outputs.
            IDXGIOutput* output;
            for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
            {
                DXGI_OUTPUT_DESC outputDesc;
                output->GetDesc(&outputDesc);

                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                GetMonitorInfo(outputDesc.Monitor, &monitorInfo);

                Monitor monitor;
                monitor.name = monitorInfo.szDevice;
                monitor.isPrimary = (monitorInfo.dwFlags == MONITORINFOF_PRIMARY);
                monitor.width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
                monitor.height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

                auto device = g_unity->Get<IUnityGraphicsD3D11>()->GetDevice();
                IDXGIOutput1* output1;
                output1 = reinterpret_cast<IDXGIOutput1*>(output);
                output1->DuplicateOutput(device, &monitor.output);

                g_monitors.push_back(monitor);

                output->Release();
            }

            adapter->Release();
        }

        factory->Release();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
        InitializeDuplication();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        g_unity = nullptr;
        FinalizeDuplication();
    }

    bool DoesMonitorExist(int id)
    {
        return id >= 0 && id < g_monitors.size();
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        if (!DoesMonitorExist(id)) return;
        auto& monitor = g_monitors[id];

        if (monitor.output == nullptr || monitor.texture == nullptr) return;

        IDXGIResource* resource = nullptr;
        DXGI_OUTDUPL_FRAME_INFO frameInfo;

        g_errorCode = monitor.output->AcquireNextFrame(g_timeout, &frameInfo, &resource);
        switch (g_errorCode) 
        {
            case S_OK: 
            {
                break;
            }
            case DXGI_ERROR_ACCESS_LOST: 
            {
                resource->Release();
                InitializeDuplication();
                return;
            }
            default: 
            {
                return;
            }
        }

        monitor.isPointerVisible = frameInfo.PointerPosition.Visible == TRUE;
        monitor.pointerX = frameInfo.PointerPosition.Position.x;
        monitor.pointerY = frameInfo.PointerPosition.Position.y;

        ID3D11Texture2D* texture;
        g_errorCode = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
        if (g_errorCode != S_OK) 
        {
            resource->Release();
            monitor.output->ReleaseFrame();
            return;
        }

        ID3D11DeviceContext* context;
        auto device = g_unity->Get<IUnityGraphicsD3D11>()->GetDevice();
        device->GetImmediateContext(&context);
        context->CopyResource(monitor.texture, texture);

        resource->Release();
        texture->Release();
        monitor.output->ReleaseFrame();
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT size_t UNITY_INTERFACE_API GetMonitorCount()
    {
        return g_monitors.size();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTimeout(int timeout)
    {
        g_timeout = timeout;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetName(int id, char* buf, int len)
    {
        if (!DoesMonitorExist(id)) return;
        strcpy_s(buf, len, g_monitors[id].name.c_str());
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsPrimary(int id)
    {
        if (!DoesMonitorExist(id)) return false;
        return g_monitors[id].isPrimary;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth(int id)
    {
        if (!DoesMonitorExist(id)) return -1;
        return g_monitors[id].width;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight(int id)
    {
        if (!DoesMonitorExist(id)) return -1;
        return g_monitors[id].height;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API IsPointerVisible(int id)
    {
        if (!DoesMonitorExist(id)) return false;
        return g_monitors[id].isPointerVisible;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerX(int id)
    {
        if (!DoesMonitorExist(id)) return -1;
        return g_monitors[id].pointerX;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerY(int id)
    {
        if (!DoesMonitorExist(id)) return -1;
        return g_monitors[id].pointerY;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, void* texture)
    {
        if (!DoesMonitorExist(id)) return;
        g_monitors[id].texture = reinterpret_cast<ID3D11Texture2D*>(texture);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetErrorCode()
    {
        return static_cast<int>(g_errorCode);
    }
}