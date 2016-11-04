#include <memory>
#include "IUnityInterface.h"

class MonitorManager;

IUnityInterfaces* GetUnity();
ID3D11Device* GetDevice();
const std::unique_ptr<MonitorManager>& GetMonitorManager();