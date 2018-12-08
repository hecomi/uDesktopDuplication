#pragma once

#include <cstdio>
#include "Debug.h"



decltype(Debug::isInitialized_) Debug::isInitialized_ = false;
decltype(Debug::mode_)          Debug::mode_ = Debug::Mode::File;
decltype(Debug::logFunc_)       Debug::logFunc_ = nullptr;
decltype(Debug::errFunc_)       Debug::errFunc_ = nullptr;
decltype(Debug::fs_)            Debug::fs_;
decltype(Debug::ss_)            Debug::ss_;
decltype(Debug::mutex_)         Debug::mutex_;


void Debug::Initialize()
{
    if (isInitialized_) return;
    isInitialized_ = true;
    
    if (mode_ == Mode::File)
    {
        fs_.open("uDesktopDuplication.log");
        Debug::Log("Start");
    }
}


void Debug::Finalize()
{
    if (!isInitialized_) return;
    isInitialized_ = false;

    if (mode_ == Mode::File)
    {
        Debug::Log("Stop");
        fs_.close();
    }
    Debug::SetLogFunc(nullptr);
    Debug::SetErrorFunc(nullptr);
}


decltype(DebugFunctionScopedTimer::currentId) DebugFunctionScopedTimer::currentId = 0;


DebugFunctionScopedTimer::DebugFunctionScopedTimer(const char* name)
    : ScopedTimer([this](std::chrono::microseconds us) { 
        Debug::Log("<< [", id_, "]", name_,  " : ", us.count(), "[us]"); 
    })
    , name_(name)
    , id_(currentId++)
{
    Debug::Log(">> [", id_, "]", name_);
}