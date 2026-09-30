#pragma once

class IKeypad
{
public:
    virtual ~IKeypad() = default;
    virtual void begin() = 0;
    virtual int readkey() = 0;
};