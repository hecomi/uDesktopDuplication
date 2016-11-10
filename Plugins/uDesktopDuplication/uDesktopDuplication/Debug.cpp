#pragma once

#include <cstdio>
#include "Debug.h"


decltype(Debug::mode_)    Debug::mode_ = Debug::Mode::kFile;
decltype(Debug::logFunc_) Debug::logFunc_ = nullptr;
decltype(Debug::errFunc_) Debug::errFunc_ = nullptr;
decltype(Debug::fs_)      Debug::fs_;


void Debug::Initialize()
{
    if (mode_ == Mode::kFile)
    {
        fs_.open("uDesktopDuplication.log");
    }
}


void Debug::Finalize()
{
    fs_.close();
}


void Debug::Log(const char* msg)
{
    switch (mode_)
    {
        case Mode::kNone:
        {
            break;
        }
        case Mode::kFile:
        {
            if (fs_.good())
            {
                fs_ << "[uDD::Log] " << msg << std::endl;
            }
            break;
        }
        case Mode::kUnityLog:
        {
            if (logFunc_ == nullptr) 
            {
                char buf[256];
                sprintf_s(buf, 256, "[uDD::Log] %s", msg);
                logFunc_(buf);
                break;
            }
        }
    }
}


void Debug::Error(const char* msg)
{
    switch (mode_)
    {
        case Mode::kNone:
        {
            break;
        }
        case Mode::kFile:
        {
            if (fs_.good())
            {
                fs_ << "[uDD::Err] " << msg << std::endl;
            }
            break;
        }
        case Mode::kUnityLog:
        {
            if (logFunc_ == nullptr) 
            {
                char buf[256];
                sprintf_s(buf, 256, "[uDD::Err] %s", msg);
                errFunc_(buf);
            }
        }
    }
}