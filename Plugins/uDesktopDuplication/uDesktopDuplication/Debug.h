#pragma once
#include <time.h>
#include <fstream>
#include <sstream>
#include "IUnityInterface.h"

// Logging
class Debug
{
public:
    enum class Mode
    {
        None = 0,
        File = 1,
        UnityLog = 2,
    };

    using DebugLogFuncPtr = void(UNITY_INTERFACE_API *)(const char*);

    static void SetMode(Mode mode) { mode_ = mode; }
    static void Initialize();
    static void Finalize();
    static void SetLogFunc(DebugLogFuncPtr func) { logFunc_ = func; }
    static void SetErrorFunc(DebugLogFuncPtr func) { errFunc_ = func; }

private:
    enum class Level
    {
        Log,
        Error
    };

    template <class T>
    static void Output(T&& arg)
    {
        if (mode_ == Mode::None) return;
        if (ss_.good())
        {
            ss_ << std::forward<T>(arg);
        }
    }

    static void Flush(Level level)
    {
        switch (mode_)
        {
            case Mode::None:
            {
                return;
            }
            case Mode::File:
            {
                if (fs_.good() && ss_.good())
                {
                    const auto str = ss_.str();
                    fs_ << str << std::endl;
                    fs_.flush();
                }
                break;
            }
            case Mode::UnityLog:
            {
                if (ss_.good())
                {
                    switch (level)
                    {
                        case Level::Log   : 
                            if (logFunc_) logFunc_(ss_.str().c_str()); 
                            break;
                        case Level::Error : 
                            if (errFunc_) errFunc_(ss_.str().c_str()); 
                            break;
                    }
                }
                break;
            }
        }
        ss_.str("");
        ss_.clear(std::stringstream::goodbit);
    }

    template <class Arg, class... RestArgs>
    static void _Log(Level level, Arg&& arg, RestArgs&&... restArgs)
    {
        Output(std::forward<Arg>(arg));
        _Log(level, std::forward<RestArgs>(restArgs)...);
    }

    static void _Log(Level level)
    {
        Flush(level);
    }

    static void OutputTime()
    {
        auto t = time(nullptr);
        tm tm;
        localtime_s(&tm, &t);
        char buf[64];
        strftime(buf, 64, "%F %T", &tm);
        Output("[");;
        Output(buf);
        Output("]");
    }

public:
    template <class Arg, class... RestArgs>
    static void Log(Arg&& arg, RestArgs&&... restArgs)
    {
        Output("[uDD::Log]");
        OutputTime();
        Output(" ");
        _Log(Level::Log, std::forward<Arg>(arg), std::forward<RestArgs>(restArgs)...);
    }

    template <class Arg, class... RestArgs>
    static void Error(Arg&& arg, RestArgs&&... restArgs)
    {
        Output("[uDD::Err]");
        OutputTime();
        Output(" ");
        _Log(Level::Error, std::forward<Arg>(arg), std::forward<RestArgs>(restArgs)...);
    }

private:
    static bool isInitialized_;
    static Mode mode_;
    static std::ofstream fs_;
    static std::ostringstream ss_;
    static DebugLogFuncPtr logFunc_;
    static DebugLogFuncPtr errFunc_;
};