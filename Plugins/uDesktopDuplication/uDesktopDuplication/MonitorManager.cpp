#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <algorithm>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"

#include "Common.h"
#include "Debug.h"
#include "Monitor.h"
#include "Cursor.h"
#include "MonitorManager.h"

using namespace Microsoft::WRL;


MonitorManager::MonitorManager()
{
    Initialize();
}


MonitorManager::~MonitorManager()
{
    Finalize();
}


void MonitorManager::Initialize()
{
    Finalize();

    // Get factory
    ComPtr<IDXGIFactory1> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    // Check all display adapters
    int id = 0;
    ComPtr<IDXGIAdapter1> adapter;
    for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
    {
        // Search the main monitor from all outputs
        ComPtr<IDXGIOutput> output;
        for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
        {
            auto monitor = std::make_shared<Monitor>(id++);
            monitor->Initialize(output.Get());
            monitors_.push_back(monitor);
        }
    }
}


void MonitorManager::Finalize()
{
    monitors_.clear();
}


void MonitorManager::RequireReinitilization()
{
    isReinitializationRequired_ = true;
}


void MonitorManager::Reinitialize()
{
    Debug::Log("MonitorManager::Reinitialize()");
    Initialize();
    SendMessageToUnity(Message::Reinitialized);
}


void MonitorManager::CheckMonitorNumbers()
{
    ComPtr<IDXGIFactory1> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    int id = 0;
    ComPtr<IDXGIAdapter1> adapter;
    for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
    {
        ComPtr<IDXGIOutput> output;
        for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
        {
            id++;
        }
    }

    if (GetMonitorCount() != id)
    {
        Debug::Log("Monitor number changed: ", GetMonitorCount(), " => ", id);
        RequireReinitilization();
    }
}


std::shared_ptr<Monitor> MonitorManager::GetMonitor(int id) const
{
    if (id >= 0 && id < monitors_.size())
    {
        return monitors_[id];
    }
    return nullptr;
}


void MonitorManager::Update()
{
    CheckMonitorNumbers();

    if (isReinitializationRequired_)
    {
        Reinitialize();
        isReinitializationRequired_ = false;
    }
}


void MonitorManager::SetTimeout(int timeout)
{
    timeout_ = timeout;
}


int MonitorManager::GetTimeout() const
{
    return timeout_;
}


int MonitorManager::GetMonitorCount() const
{
    return static_cast<int>(monitors_.size());
}

int MonitorManager::GetTotalWidth() const
{
    std::vector<int> lefts, rights;
    for (const auto& monitor : monitors_)
    {
        lefts.push_back(monitor->GetLeft());
        rights.push_back(monitor->GetRight());
    }
    const auto minLeft = *std::min_element(lefts.begin(), lefts.end());
    const auto maxRight = *std::max_element(rights.begin(), rights.end());
    return maxRight - minLeft;
}

int MonitorManager::GetTotalHeight() const
{
    std::vector<int> tops, bottoms;
    for (const auto& monitor : monitors_)
    {
        tops.push_back(monitor->GetTop());
        bottoms.push_back(monitor->GetBottom());
    }
    const auto minTop = *std::min_element(tops.begin(), tops.end());
    const auto maxBottom = *std::max_element(bottoms.begin(), bottoms.end());
    return maxBottom - minTop;
}