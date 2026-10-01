#pragma once

class IAccessIndicator
{
public:
    virtual void begin() = 0;
    virtual void locked() = 0;
    virtual void unlocked() = 0;
    virtual void accessDenied() = 0;
    virtual void off() = 0;
};