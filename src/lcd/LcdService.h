#pragma once

#include "ILcd.h"
#include "../config/AppConfig.h"
#include "LiquidCrystal_I2C.h"

class LcdService : public ILcd
{
private:
    LiquidCrystal_I2C lcd;

public:
    LcdService();

    void begin() override;
    void clear() override;
    void print(const char *text) override;
};
