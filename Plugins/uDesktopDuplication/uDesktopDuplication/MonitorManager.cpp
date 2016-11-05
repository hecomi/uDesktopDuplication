#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <algorithm>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"

#include "Common.h"
#include "Monitor.h"
#include "Cursor.h"
#include "MonitorManager.h"


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
    IDXGIFactory1* factory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));

    // Check all display adapters
    int id = 0;
    IDXGIAdapter1* adapter;
    for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
    {
        // Search the main monitor from all outputs
        IDXGIOutput* output;
        for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
        {
            auto monitor = std::make_shared<Monitor>(id++);
            monitor->Initialize(output);
            monitors_.push_back(monitor);
            output->Release();
        }
        adapter->Release();
    }
    factory->Release();
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
	Initialize();
	SendMessageToUnity(Message::Reinitialized);
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