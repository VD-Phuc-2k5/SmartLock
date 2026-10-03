#include <Arduino.h>
#include <cstring>
#include "config/AppConfig.h"
#include "config/ConfigManager.h"
#include "keypad/KeypadService.h"
#include "lcd/LcdService.h"
#include "mqtt/MqttClient.h"
#include "otp/OtpService.h"
#include "rfid/RfidService.h"
#include "indicator/AccessIndicator.h"
#include "state/LockController.h"
#include "door/DoorLock.h"
#include "web/WebConfigService.h"

ConfigManager configManager;
KeypadService keypad;
LcdService lcd;
WebConfigService webConfig(configManager);
MqttClient mqtt(configManager);
OtpService otp;
RfidService rfid;
AccessIndicator indicator;
DoorLock doorLock;
LockController controller(
    lcd,
    mqtt,
    otp,
    indicator,
    doorLock);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========== BOOT ==========");

    configManager.begin();

    if (!configManager.hasConfig())
    {
        Serial.println("[MAIN] No network configuration");
        Serial.println("[MAIN] Starting configuration portal...");

        if (!webConfig.begin())
        {
            Serial.println("[MAIN] Failed to start web config");
            return;
        }

        Serial.println("[MAIN] =============================");
        Serial.println("[MAIN] Connect to Wi-Fi:");
        Serial.println("[MAIN]     SmartLock-Setup");
        Serial.println("[MAIN] Then open:");
        Serial.println("[MAIN]     http://192.168.4.1");
        Serial.println("[MAIN] =============================");

        return;
    }

    Serial.println("[MAIN] Network configuration found");

    const NetworkConfig &networkConfig = configManager.get();

    Serial.print("[MAIN] Wi-Fi SSID: ");
    Serial.println(networkConfig.ssid);

    Serial.print("[MAIN] MQTT Broker: ");
    Serial.print(networkConfig.mqttHost);
    Serial.print(":");
    Serial.println(networkConfig.mqttPort);

    keypad.begin();
    lcd.begin();

    rfid.begin();
    indicator.begin();
    doorLock.begin();

    controller.setState(
        &controller.idleState);

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
            bool valid =
                length == 5 &&
                std::strncmp(
                    payload,
                    "valid",
                    5) == 0;

            controller.onVerifyResult(valid);
        });

    mqtt.subscribe(
        AppConfig::Topics::CARD_ENROLL_RESULT,
        [](const char *payload, unsigned int length)
        {
            bool ok =
                length == 2 &&
                std::strncmp(
                    payload,
                    "ok",
                    2) == 0;

            controller.onEnrollResult(ok);
        });

    mqtt.begin();

    Serial.println("[MAIN] Smart Lock started");
}

void loop()
{
    if (!configManager.hasConfig())
    {
        webConfig.handleClient();
        delay(2);

        return;
    }

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
        Serial.print("Keypad key: ");
        Serial.println(key);

        controller.onKey(key);
    }
}
