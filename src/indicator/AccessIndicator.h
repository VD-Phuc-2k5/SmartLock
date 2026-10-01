#pragma once

#include "IAccessIndicator.h"

class AccessIndicator : public IAccessIndicator
{
public:
    void begin();
    void locked();
    void unlocked();
    void accessDenied();
    void off();

private:
    void beep(unsigned long duration);
};