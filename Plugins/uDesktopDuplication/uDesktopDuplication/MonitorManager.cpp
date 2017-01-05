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


MonitorManager::MonitorManager(LUID unityAdapterLuid)
	: unityAdapterLuid_(unityAdapterLuid)
{
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
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        Debug::Error("MonitorManager::Initialize() => CreateDXGIFactory1() failed.");
        return;
    }

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
            const auto unityAdapterLuid = GetUnityAdapterLuid();
            monitor->Initialize(adapter, output);
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


bool MonitorManager::HasMonitorCountChanged() const
{
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        Debug::Error("MonitorManager::CheckMonitorConnection() => CreateDXGIFactory1() failed.");
        return false;
    }

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

    return monitors_.size() != id;
}


std::shared_ptr<Monitor> MonitorManager::GetMonitor(int id) const
{
	if(monitors_.empty()) {
		const_cast<MonitorManager*>(this)->Initialize();
	}
    if (id >= 0 && id < static_cast<int>(monitors_.size()))
    {
        return monitors_[id];
    }
    return nullptr;
}


std::shared_ptr<Cursor> MonitorManager::GetCursor() const
{
    return cursor_;
}


void MonitorManager::Update()
{
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
	if (monitors_.empty()) {
		const_cast<MonitorManager*>(this)->Initialize();
	}
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
