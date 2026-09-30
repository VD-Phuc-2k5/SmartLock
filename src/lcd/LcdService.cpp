#include "../config/PinConfig.h"
#include "LcdService.h"
#include <Wire.h>

LcdService::LcdService() : lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS) {}

void LcdService::begin()
{
    Wire.begin(PinConfig::Lcd::SDA, PinConfig::Lcd::SCL);

    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void LcdService::clear()
{
    lcd.clear();
}

void LcdService::print(const char *text)
{
    lcd.print(text);
}