#include <memory>
#include "IUnityInterface.h"

class MonitorManager;

IUnityInterfaces* GetUnity();
ID3D11Device* GetDevice();
const std::unique_ptr<MonitorManager>& GetMonitorManager();

enum class Message
{
    None = -1,
    Reinitialized = 0,
};
void SendMessageToUnity(Message message);