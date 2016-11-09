#pragma once

#include <cstdio>
#include "Debug.h"


#define INIT_STATIC_MEMBER(Member, Value) \
    decltype(Debug::Member) Debug::Member = Value;
INIT_STATIC_MEMBER(enabled_, true)
INIT_STATIC_MEMBER(logFunc_, nullptr)
INIT_STATIC_MEMBER(errFunc_, nullptr)


void Debug::Log(const char* msg)
{
    if (!enabled_ || logFunc_ == nullptr) return;

    char buf[256];
    sprintf_s(buf, 256, "[uDD::Log] %s", msg);
    logFunc_(buf);
}


void Debug::Error(const char* msg)
{
    if (!enabled_ || errFunc_ == nullptr) return;

    char buf[256];
    sprintf_s(buf, 256, "[uDD::Err] %s", msg);
    errFunc_(buf);
}