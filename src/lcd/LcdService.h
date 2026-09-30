#pragma once

#include "ILcd.h"
#include "LiquidCrystal_I2C.h"

class LcdService : public ILcd
{
private:
    static constexpr uint8_t LCD_ADDRESS = 0x27;
    static constexpr uint8_t LCD_COLUMNS = 16;
    static constexpr uint8_t LCD_ROWS = 2;

    LiquidCrystal_I2C lcd;

public:
    LcdService();

    void begin() override;
    void clear() override;
    void print(const char *text) override;
};