#pragma once

class ILcd
{
public:
    virtual ~ILcd() = default;

    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void print(const char *text) = 0;
};