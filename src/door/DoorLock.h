#pragma once

#include "IDoorLock.h"

class DoorLock : public IDoorLock
{
public:
    void begin();

    void lock();
    void unlock();

private:
    static constexpr int LOCK_POSITION = 0;
    static constexpr int UNLOCK_POSITION = 90;
};