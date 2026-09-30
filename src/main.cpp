#include <Arduino.h>
#include "config/AppConfig.h"
#include "keypad/KeypadService.h"
#include "lcd/LcdService.h"
#include "mqtt/MqttClient.h"
#include "otp/OtpService.h"

KeypadService keypad;
LcdService lcd;
MqttClient mqtt;
OtpService otp;

namespace
{
    char inputOtp[AppConfig::Otp::LENGTH + 1] = {};
    uint8_t inputLength = 0;
}

void resetInput()
{
    inputLength = 0;
    inputOtp[0] = '\0';
}

void setup()
{
    Serial.begin(115200);
    keypad.begin();
    lcd.begin();
    lcd.print("Waiting ...");
    mqtt.begin();

    mqtt.subscribe(AppConfig::Topics::OTP, [](const char *payload, unsigned int length)
    {
        otp.setOtp(payload, length);
        lcd.clear();
        lcd.print("OTP received");
    });
}

void loop()
{
    mqtt.loop();

    char key = keypad.readkey();
    if (key == NO_KEY)
    {
        return;
    }

    if (key >= '0' && key <= '9')
    {
        if (inputLength < AppConfig::Otp::LENGTH)
        {
            inputOtp[inputLength++] = key;
            inputOtp[inputLength] = '\0';

            lcd.clear();
            lcd.print(inputOtp);
        }
    }
    else if (key == '*')
    {
        resetInput();
        lcd.clear();
        lcd.print("Cleared");
    }
    else if (key == '#')
    {
        if (otp.verify(inputOtp))
        {
            lcd.clear();
            lcd.print("OTP VALID");
        }
        else
        {
            lcd.clear();
            lcd.print("OTP INVALID");
        }

        resetInput();
    }
}
