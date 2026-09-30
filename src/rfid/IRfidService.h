#pragma once

#include <Arduino.h>

class IRfidService
{
public:
    virtual ~IRfidService() = default;

    virtual void begin() = 0;
    virtual bool isCardPresent() = 0;
    virtual String readUid() = 0;
};
