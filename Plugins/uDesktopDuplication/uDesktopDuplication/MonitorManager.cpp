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
}


MonitorManager::~MonitorManager()
{
}


void MonitorManager::Initialize()
{
    UDD_FUNCTION_SCOPE_TIMER

    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        Debug::Error("MonitorManager::Initialize() => CreateDXGIFactory1() failed.");
        return;
    }

    std::vector<std::pair<ComPtr<IDXGIAdapter1>, ComPtr<IDXGIOutput>>> outputs;

    ComPtr<IDXGIAdapter1> adapter;
    for (int i = 0; (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND); ++i) 
    {
        DXGI_ADAPTER_DESC desc;
        if (FAILED(adapter->GetDesc(&desc))) continue;
        Debug::Log("Graphics Card [", i, "] : ", desc.Description);

        ComPtr<IDXGIOutput> output;
        for (int j = 0; (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND); ++j) 
        {
            DXGI_OUTPUT_DESC desc;
            if (FAILED(output->GetDesc(&desc))) continue;
            Debug::Log("  > Monitor[", j, "] : ", desc.DeviceName);
            outputs.emplace_back(adapter, output);
        }
    }

    for (int id = 0; id < static_cast<int>(outputs.size()); ++id)
    {
        const auto& pair = outputs.at(id);
        auto monitor = std::make_shared<Monitor>(id);
        monitor->Initialize(pair.first, pair.second);
        monitor->StartCapture();
        monitors_.push_back(monitor);
    }
}


void MonitorManager::Finalize()
{
    UDD_FUNCTION_SCOPE_TIMER

    for (const auto& monitor : monitors_)
    {
        monitor->Finalize();
    }

    monitors_.clear();
}


void MonitorManager::Update()
{
    UDD_FUNCTION_SCOPE_TIMER

    if (isReinitializationRequired_)
    {
        Reinitialize();
        isReinitializationRequired_ = false;
    }
}


void MonitorManager::RequireReinitilization()
{
    Debug::Log("MonitorManager::Reinitialize() was required.");
    isReinitializationRequired_ = true;
}


void MonitorManager::Reinitialize()
{
    UDD_FUNCTION_SCOPE_TIMER

    Debug::Log("MonitorManager::Reinitialize()");
    Finalize();
    Initialize();
    SendMessageToUnity(Message::Reinitialized);
}


bool MonitorManager::HasMonitorCountChanged() const
{
    UDD_FUNCTION_SCOPE_TIMER

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


void MonitorManager::SetFrameRate(UINT frameRate)
{
    frameRate_ = frameRate;
}


UINT MonitorManager::GetFrameRate() const
{
    return frameRate_;
}
