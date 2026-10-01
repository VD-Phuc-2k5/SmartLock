#include <Arduino.h>
#include <cstring>
#include "config/AppConfig.h"
#include "keypad/KeypadService.h"
#include "lcd/LcdService.h"
#include "mqtt/MqttClient.h"
#include "otp/OtpService.h"
#include "rfid/RfidService.h"
#include "indicator/AccessIndicator.h"
#include "state/LockController.h"
#include "door/DoorLock.h"

KeypadService keypad;
LcdService lcd;
MqttClient mqtt;
OtpService otp;
RfidService rfid;
AccessIndicator indicator;
DoorLock doorLock;

LockController controller(lcd, mqtt, otp, indicator, doorLock);

void setup()
{
    Serial.begin(115200);
    keypad.begin();
    lcd.begin();
    controller.setState(&controller.idleState);

    mqtt.begin();
    rfid.begin();
    indicator.begin();
    doorLock.begin();

    mqtt.subscribe(
        AppConfig::Topics::OTP,
        [](const char *payload, unsigned int length)
        {
            otp.setOtp(payload, length);
            controller.onOtpReceived();
        });

    mqtt.subscribe(
        AppConfig::Topics::CARD_VERIFY_RESULT,
        [](const char *payload, unsigned int length)
        {
            bool valid = (length == 5 && std::strncmp(payload, "valid", 5) == 0);
            controller.onVerifyResult(valid);
        });

    mqtt.subscribe(
        AppConfig::Topics::CARD_ENROLL_RESULT,
        [](const char *payload, unsigned int length)
        {
            bool ok = (length == 2 && std::strncmp(payload, "ok", 2) == 0);
            controller.onEnrollResult(ok);
        });
}

void loop()
{
    mqtt.loop();

    if (rfid.isCardPresent())
    {
        String uid = rfid.readUid();

        Serial.print("RFID UID: ");
        Serial.println(uid);

        controller.onCard(uid);
    }

    char key = keypad.readkey();
    if (key != NO_KEY)
    {
        controller.onKey(key);
    }
}
