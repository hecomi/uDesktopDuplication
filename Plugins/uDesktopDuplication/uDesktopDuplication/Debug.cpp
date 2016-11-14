#pragma once

#include <cstdio>
#include "Debug.h"


decltype(Debug::mode_)    Debug::mode_ = Debug::Mode::File;
decltype(Debug::logFunc_) Debug::logFunc_ = nullptr;
decltype(Debug::errFunc_) Debug::errFunc_ = nullptr;
decltype(Debug::fs_)      Debug::fs_;
decltype(Debug::ss_)      Debug::ss_;


void Debug::Initialize()
{
    if (mode_ == Mode::File)
    {
        fs_.open("uDesktopDuplication.log");
        Debug::Log("Start");
    }
}


void Debug::Finalize()
{
    if (mode_ == Mode::File)
    {
        Debug::Log("Stop");
        fs_.close();
    }
	Debug::SetLogFunc(nullptr);
	Debug::SetErrorFunc(nullptr);
}