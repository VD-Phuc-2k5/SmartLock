#pragma once

class IDoorLock
{
public:
    virtual void begin() = 0;

    virtual void lock() = 0;
    virtual void unlock() = 0;
};