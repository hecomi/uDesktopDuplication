#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"

#include "Common.h"
#include "MonitorManager.h"

#pragma comment(lib, "dxgi.lib")


namespace
{
    IUnityInterfaces* g_unity = nullptr;
    std::unique_ptr<MonitorManager> g_manager;
}


IUnityInterfaces* GetUnity()
{
    return g_unity;
}


ID3D11Device* GetDevice()
{
    return GetUnity()->Get<IUnityGraphicsD3D11>()->GetDevice();
}

const std::unique_ptr<MonitorManager>& GetMonitorManager()
{
    return g_manager;
}


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
        g_manager = std::make_unique<MonitorManager>();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        g_manager.reset();
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        g_manager->OnRender(id);
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT size_t UNITY_INTERFACE_API GetMonitorCount()
    {
        return g_manager->GetMonitorCount();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTimeout(int timeout)
    {
        g_manager->SetTimeout(timeout);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetName(int id, char* buf, int len)
    {
        g_manager->GetName(id, buf, len);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsPrimary(int id)
    {
        return g_manager->IsPrimary(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth(int id)
    {
        return g_manager->GetWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight(int id)
    {
        return g_manager->GetHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API IsCursorVisible(int id)
    {
        return g_manager->IsCursorVisible(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorX(int id)
    {
        return g_manager->GetCursorX(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorY(int id)
    {
        return g_manager->GetCursorY(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeWidth(int id)
    {
        return g_manager->GetCursorShapeWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeHeight(int id)
    {
        return g_manager->GetCursorShapeHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapePitch(int id)
    {
        return g_manager->GetCursorShapePitch(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeType(int id)
    {
        return g_manager->GetCursorShapeType(id);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UpdateCursorTexture(int id, ID3D11Texture2D* ptr)
    {
        g_manager->UpdateCursorTexture(id, ptr);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, void* texture)
    {
        g_manager->SetTexturePtr(id, texture);
    }
}