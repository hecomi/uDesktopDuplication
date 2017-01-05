#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>

struct IUnityInterfaces;
class Monitor;
class Cursor;

class MonitorManager
{
public:
    explicit MonitorManager(LUID unityAdapterLuid_);
    ~MonitorManager();
    void Reinitialize();
    bool HasMonitorCountChanged() const;
    void RequireReinitilization();
    void SetCursorMonitorId(int id) { cursorMonitorId_ = id; }
    int GetCursorMonitorId() const { return cursorMonitorId_; }
    std::shared_ptr<Monitor> GetMonitor(int id) const;
    std::shared_ptr<Cursor> GetCursor() const;

private:
	void Initialize();
    void Finalize();

// Setters from Unity
public:
    void Update();
    void SetTimeout(int timeout);
    int GetTimeout() const;

// Getters from Unity
public:
    int GetMonitorCount() const;
    int GetTotalWidth() const;
    int GetTotalHeight() const;

private:
	LUID unityAdapterLuid_;

    int timeout_ = 10;
    bool enableTextureCopyFromGpuToCpu_ = false;
    std::vector<std::shared_ptr<Monitor>> monitors_;
    std::shared_ptr<Cursor> cursor_ = std::make_shared<Cursor>();
    int cursorMonitorId_ = -1;
    bool isReinitializationRequired_ = false;
};
