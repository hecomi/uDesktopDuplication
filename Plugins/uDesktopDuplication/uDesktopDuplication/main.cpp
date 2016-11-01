#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "Duplication.h"

#pragma comment(lib, "dxgi.lib")


namespace
{
    std::unique_ptr<Duplication> g_dupl;
}


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_dupl = std::make_unique<Duplication>(unityInterfaces);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        g_dupl.reset();
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        g_dupl->OnRender(id);
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT size_t UNITY_INTERFACE_API GetMonitorCount()
    {
        return g_dupl->GetMonitorCount();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTimeout(int timeout)
    {
        g_dupl->SetTimeout(timeout);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetName(int id, char* buf, int len)
    {
        g_dupl->GetName(id, buf, len);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsPrimary(int id)
    {
        return g_dupl->IsPrimary(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth(int id)
    {
        return g_dupl->GetWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight(int id)
    {
        return g_dupl->GetHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API IsPointerVisible(int id)
    {
        return g_dupl->IsPointerVisible(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerX(int id)
    {
        return g_dupl->GetPointerX(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerY(int id)
    {
        return g_dupl->GetPointerY(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerShapeWidth(int id)
    {
        return g_dupl->GetPointerShapeWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerShapeHeight(int id)
    {
        return g_dupl->GetPointerShapeHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerShapePitch(int id)
    {
        return g_dupl->GetPointerShapePitch(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetPointerShapeType(int id)
    {
        return g_dupl->GetPointerShapeType(id);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UpdatePointerTexture(int id, ID3D11Texture2D* ptr)
    {
        g_dupl->UpdatePointerTexture(id, ptr);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, void* texture)
    {
        g_dupl->SetTexturePtr(id, texture);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetErrorCode()
    {
        return g_dupl->GetErrorCode();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetErrorMessage(char* buf, int len)
    {
        g_dupl->GetErrorMessage(buf, len);
    }

}