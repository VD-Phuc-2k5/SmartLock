#include <Arduino.h>
#include "keypad/KeypadService.h"

KeypadService keypad;

void setup()
{
    Serial.begin(115200);
    keypad.begin();
}

void loop()
{
    int key = keypad.readkey();
    if (key != NO_KEY)
    {
        Serial.print("Key pressed: ");
        Serial.println(key);
    }
}