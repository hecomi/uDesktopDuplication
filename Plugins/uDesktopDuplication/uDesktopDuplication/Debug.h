#pragma once

// Logging
class Debug
{
public:
    static void Enable() { enabled_ = true; }
    static void Disable() { enabled_ = false; }

    using DebugLogFuncPtr = void(*)(const char*);
    static void SetLogFunc(DebugLogFuncPtr func) { logFunc_ = func; }
    static void SetErrorFunc(DebugLogFuncPtr func) { errFunc_ = func; }

public:
    static void Log(const char* msg);
    static void Error(const char* msg);

private:
    static bool enabled_;
    static DebugLogFuncPtr logFunc_;
    static DebugLogFuncPtr errFunc_;
};