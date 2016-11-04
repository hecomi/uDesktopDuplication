#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>
#include <queue>

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
	std::queue<Message> g_messages;
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


void SendMessageToUnity(Message message)
{
	g_messages.push(message);
}


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API InitializeUDD()
    {
		if (g_unity && !g_manager)
		{
			g_manager = std::make_unique<MonitorManager>();
		}
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API FinalizeUDD()
    {
		g_manager.reset();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
		InitializeUDD();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
		g_unity = nullptr;
		FinalizeUDD();
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
		if (!g_manager) return;
        g_manager->OnRender(id);
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Update()
    {
		if (!g_manager) return;
		g_manager->Update();
    }

	UNITY_INTERFACE_EXPORT Message PopMessage()
	{
		if (g_messages.empty()) return Message::None;

		const auto message = g_messages.front();
		g_messages.pop();
		return message;
	}

    UNITY_INTERFACE_EXPORT size_t UNITY_INTERFACE_API GetMonitorCount()
    {
		if (!g_manager) return 0;
        return g_manager->GetMonitorCount();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetTotalWidth()
    {
		if (!g_manager) return 0;
        return g_manager->GetTotalWidth();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetTotalHeight()
    {
		if (!g_manager) return 0;
        return g_manager->GetTotalHeight();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTimeout(int timeout)
    {
		if (!g_manager) return;
        g_manager->SetTimeout(timeout);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetName(int id, char* buf, int len)
    {
		if (!g_manager) return;
        g_manager->GetName(id, buf, len);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsPrimary(int id)
    {
		if (!g_manager) return false;
        return g_manager->IsPrimary(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetLeft(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetLeft(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetRight(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetRight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetTop(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetTop(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetBottom(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetBottom(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetRotation(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetRotation(id);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsCursorVisible(int id)
    {
		if (!g_manager) return false;
        return g_manager->IsCursorVisible(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorX(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorX(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorY(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorY(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeWidth(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorShapeWidth(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeHeight(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorShapeHeight(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapePitch(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorShapePitch(id);
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeType(int id)
    {
		if (!g_manager) return -1;
        return g_manager->GetCursorShapeType(id);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UpdateCursorTexture(int id, ID3D11Texture2D* ptr)
    {
		if (!g_manager) return;
        g_manager->UpdateCursorTexture(id, ptr);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, void* texture)
    {
		if (!g_manager) return;
        g_manager->SetTexturePtr(id, texture);
    }
}