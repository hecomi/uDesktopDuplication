#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>
#include <memory>

struct IUnityInterfaces;
class Monitor;
class Cursor;

class MonitorManager final
{
public:
    MonitorManager();
    ~MonitorManager();
	void Initialize();
    void Finalize();
    void Reinitialize();
    void Update();
    bool HasMonitorCountChanged() const;
    void RequireReinitilization();
    void SetCursorMonitorId(int id) { cursorMonitorId_ = id; }
    int GetCursorMonitorId() const { return cursorMonitorId_; }
    std::shared_ptr<Monitor> GetMonitor(int id) const;
    std::shared_ptr<Cursor> GetCursor() const;
    void SetFrameRate(UINT frameRate);
    UINT GetFrameRate() const;

public:
    int GetMonitorCount() const;
    int GetTotalWidth() const;
    int GetTotalHeight() const;

private:
    UINT frameRate_ = 60;
    bool enableTextureCopyFromGpuToCpu_ = false;
    std::vector<std::shared_ptr<Monitor>> monitors_;
    std::shared_ptr<Cursor> cursor_ = std::make_shared<Cursor>();
    int cursorMonitorId_ = -1;
    bool isReinitializationRequired_ = false;
};
