#pragma once

#include <memory>
#include <wrl/client.h>


// Unity interface and ID3D11Device getters
struct IUnityInterfaces;
IUnityInterfaces* GetUnity();

struct ID3D11Device;
Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();


// Manager getter
class MonitorManager;
const std::unique_ptr<MonitorManager>& GetMonitorManager();


// Message is pooled and fetch from Unity.
enum class Message
{
    None = -1,
    Reinitialized = 0,
	TextureSizeChanged = 1,
};

void SendMessageToUnity(Message message);