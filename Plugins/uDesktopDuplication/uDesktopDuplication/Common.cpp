#pragma once

#include <queue>
#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Common.h"

using namespace Microsoft::WRL;


extern IUnityInterfaces* g_unity;
extern std::unique_ptr<MonitorManager> g_manager;
extern std::queue<Message> g_messages;


IUnityInterfaces* GetUnity()
{
    return g_unity;
}


ComPtr<ID3D11Device> GetDevice()
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
