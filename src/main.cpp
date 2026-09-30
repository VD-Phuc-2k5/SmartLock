#include <Arduino.h>
#include "keypad/KeypadService.h"
#include "lcd/LcdService.h"

KeypadService keypad;
LcdService lcd;

void setup()
{
    Serial.begin(115200);
    keypad.begin();
    lcd.begin();
}

void loop()
{
    char key = keypad.readkey();
    if (key != NO_KEY)
    {
        lcd.clear();
        lcd.print("Key: ");
        lcd.print(&key);
    }
}