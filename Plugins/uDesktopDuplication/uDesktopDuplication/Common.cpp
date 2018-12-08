#pragma once

#include <queue>
#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Common.h"
#include "Debug.h"

using namespace Microsoft::WRL;



extern IUnityInterfaces* g_unity;
extern std::unique_ptr<MonitorManager> g_manager;
extern std::queue<Message> g_messages;


void OutputWindowsInformation()
{
	const auto hModule = ::LoadLibrary(TEXT("ntdll.dll"));
	if (!hModule) return;

    ScopedReleaser freeModule([&] { ::FreeLibrary(hModule); });

	if (const auto address = ::GetProcAddress(hModule, "RtlGetVersion"))
	{
		using RtlGetVersionType = NTSTATUS(WINAPI *)(OSVERSIONINFOEXW*);
		const auto RtlGetVersion = reinterpret_cast<RtlGetVersionType>(address);

		OSVERSIONINFOEXW os = { sizeof(os) };
		if (!FAILED(RtlGetVersion(&os)))
		{
			Debug::Log("OS Version    : ", os.dwMajorVersion, ".", os.dwMinorVersion);
			Debug::Log("Build Number  : ", os.dwBuildNumber);
			Debug::Log("Service Pack  : ", os.szCSDVersion);
		}
	}
}


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


LUID GetUnityAdapterLuid()
{
    UDD_FUNCTION_SCOPE_TIMER

    const auto device = GetDevice();

    Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))){
        Debug::Error("QueryInterface from IUnityGraphicsD3D11 to IDXGIDevice1 failed.");
        return LUID();
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    if (FAILED(dxgiDevice->GetAdapter(&dxgiAdapter))) {
        Debug::Error("QueryInterface from IDXGIDevice1 to IDXGIAdapter failed.");
        return LUID();
    }

    DXGI_ADAPTER_DESC adapterDesc;
    dxgiAdapter->GetDesc(&adapterDesc);

    return adapterDesc.AdapterLuid;
}


void SendMessageToUnity(Message message)
{
    g_messages.push(message);
}


ScopedTimer::ScopedTimer(TimerFuncType&& func)
    : func_(func)
    , start_(std::chrono::high_resolution_clock::now())
{
}


ScopedTimer::~ScopedTimer()
{
    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<microseconds>(end - start_);
    func_(time);
}
