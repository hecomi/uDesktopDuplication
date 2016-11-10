#pragma once
#include <fstream>
#include "IUnityInterface.h"

// Logging
class Debug
{
public:
    enum class Mode
    {
        kNone = 0,
        kFile = 1,
        kUnityLog = 2,
    };

    using DebugLogFuncPtr = void(UNITY_INTERFACE_API *)(const char*);

    static void SetMode(Mode mode) { mode_ = mode; }
    static void Initialize();
    static void Finalize();
    static void SetLogFunc(DebugLogFuncPtr func) { logFunc_ = func; }
    static void SetErrorFunc(DebugLogFuncPtr func) { errFunc_ = func; }

public:
    static void Log(const char* msg);
    static void Error(const char* msg);

private:
    static Mode mode_;
    static std::ofstream fs_;
    static DebugLogFuncPtr logFunc_;
    static DebugLogFuncPtr errFunc_;
};