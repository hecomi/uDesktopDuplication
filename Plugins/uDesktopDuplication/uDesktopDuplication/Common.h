#pragma once

#include <memory>
#include <string>
#include <functional>


// Unity interface and ID3D11Device getters
struct IUnityInterfaces;
IUnityInterfaces* GetUnity();

struct ID3D11Device;
ID3D11Device* GetDevice();


// Utility
template <class T>
auto MakeUniqueWithReleaser(T* ptr)
{
    const auto deleter = [](T* ptr) { ptr->Release(); };
    return std::unique_ptr<T, decltype(deleter)>(ptr, deleter);
}


// Manager getter
class MonitorManager;
const std::unique_ptr<MonitorManager>& GetMonitorManager();


// Message is pooled and fetch from Unity.
enum class Message
{
    None = -1,
    Reinitialized = 0,
};

void SendMessageToUnity(Message message);