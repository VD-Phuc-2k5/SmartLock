#include <Arduino.h>
#include "keypad/KeypadService.h"
#include "lcd/LcdService.h"
#include "mqtt/MqttClient.h"

KeypadService keypad;
LcdService lcd;
MqttClient mqtt;

void setup()
{
    Serial.begin(115200);
    keypad.begin();
    lcd.begin();
    lcd.print("Waiting ...");
    mqtt.begin();
}

void loop()
{
    mqtt.loop();

    if (mqtt.hasMessage())
    {
        lcd.clear();
        lcd.print(mqtt.getMessage());
        mqtt.clearMessage();
    }

    char key = keypad.readkey();
    if (key != NO_KEY)
    {
        lcd.clear();
        lcd.print("Key: ");
        lcd.print(&key);
    }
}