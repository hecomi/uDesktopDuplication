#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>
#include <queue>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

#include "Common.h"
#include "Debug.h"
#include "Monitor.h"
#include "Cursor.h"
#include "MonitorManager.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shcore.lib")


IUnityInterfaces* g_unity = nullptr;
std::unique_ptr<MonitorManager> g_manager;
std::queue<Message> g_messages;


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Initialize()
    {
        if (g_unity && !g_manager)
        {
            Debug::Initialize();
            g_manager = std::make_unique<MonitorManager>();
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Finalize()
    {
        if (!g_manager) return;

        g_manager.reset();

        std::queue<Message> empty;
        g_messages.swap(empty);

        Debug::Finalize();
    }

    void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType event)
    {
        switch (event)
        {
            case kUnityGfxDeviceEventInitialize:
            {
                Initialize();
                break;
            }
            case kUnityGfxDeviceEventShutdown:
            {
                Finalize();
                break;
            }
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
        auto unityGraphics = g_unity->Get<IUnityGraphics>();
        unityGraphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        auto unityGraphics = g_unity->Get<IUnityGraphics>();
        unityGraphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
        g_unity = nullptr;
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        if (!g_manager) return;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            monitor->Render(g_manager->GetTimeout());
        }
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Reinitialize()
    {
        if (!g_manager) return;
        return g_manager->Reinitialize();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Update()
    {
        if (!g_manager) return;
        g_manager->Update();
    }

    UNITY_INTERFACE_EXPORT Message UNITY_INTERFACE_API PopMessage()
    {
        if (g_messages.empty()) return Message::None;

        const auto message = g_messages.front();
        g_messages.pop();
        return message;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetDebugMode(Debug::Mode mode)
    {
        Debug::SetMode(mode);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetLogFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetLogFunc(func);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetErrorFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetErrorFunc(func);
    }

    UNITY_INTERFACE_EXPORT size_t UNITY_INTERFACE_API GetMonitorCount()
    {
        if (!g_manager) return 0;
        return g_manager->GetMonitorCount();
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API HasMonitorCountChanged()
    {
        if (!g_manager) return false;
        return g_manager->HasMonitorCountChanged();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorMonitorId()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursorMonitorId();
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

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetId(int id)
    {
        if (!g_manager) return;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            monitor->GetId();
        }
    }

    UNITY_INTERFACE_EXPORT MonitorState UNITY_INTERFACE_API GetState(int id)
    {
        if (!g_manager) return MonitorState::NotSet;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetState();
        }
        return MonitorState::NotSet;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetName(int id, char* buf, int len)
    {
        if (!g_manager) return;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            monitor->GetName(buf, len);
        }
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsPrimary(int id)
    {
        if (!g_manager) return false;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->IsPrimary();
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetLeft(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetLeft();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetRight(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetRight();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetTop(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetTop();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetBottom(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetBottom();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetWidth();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetHeight();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetRotation(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetRotation();
        }
        return DXGI_MODE_ROTATION_UNSPECIFIED;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetDpiX(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetDpiX();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetDpiY(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetDpiY();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API IsCursorVisible()
    {
        if (!g_manager) return false;
        return g_manager->GetCursor()->IsVisible();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorX()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetX();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorY()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetY();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeWidth()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetWidth();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeHeight()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetHeight();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapePitch()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetPitch();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetCursorShapeType()
    {
        if (!g_manager) return -1;
        return g_manager->GetCursor()->GetType();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetCursorTexture(ID3D11Texture2D* texture)
    {
        if (!g_manager) return;
        return g_manager->GetCursor()->GetTexture(texture);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, void* texture)
    {
        if (!g_manager) return;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            auto d3d11Texture = reinterpret_cast<ID3D11Texture2D*>(texture);
            monitor->SetUnityTexture(d3d11Texture);
        }
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetMoveRectCount(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetMoveRectCount();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API GetMoveRects(int id)
    {
        if (!g_manager) return nullptr;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetMoveRects();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetDirtyRectCount(int id)
    {
        if (!g_manager) return -1;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetDirtyRectCount();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API GetDirtyRects(int id)
    {
        if (!g_manager) return nullptr;
        if (auto monitor = g_manager->GetMonitor(id))
        {
            return monitor->GetDirtyRects();
        }
        return nullptr;
    }
}