#include <Arduino.h>
#include <cstring>
#include <WiFi.h>

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

// ============================================================
// GLOBAL SERVICES
// ============================================================

ConfigManager config;

KeypadService keypad;
LcdService lcd;

MqttClient mqtt(config);

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

WebConfigService webConfig(config);

// ============================================================
// WIFI
// ============================================================

bool connectWifi()
{
    const NetworkConfig &networkConfig = config.get();

    Serial.println("[WIFI] Connecting...");
    Serial.print("[WIFI] SSID: ");
    Serial.println(networkConfig.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(
        networkConfig.ssid.c_str(),
        networkConfig.password.c_str());

    unsigned long start = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - start < AppConfig::Wifi::TIMEOUT_MS)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WIFI] Connection FAILED");
        return false;
    }

    Serial.println("[WIFI] Connected");

    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());

    return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(5000);

    Serial.println();
    Serial.println("================================");
    Serial.println("       SMART LOCK BOOT");
    Serial.println("================================");

    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    Serial.println("[SETUP] 1. ConfigManager");

    bool hasConfig = config.begin();

    if (hasConfig)
    {
        Serial.println("[SETUP] Network configuration loaded");

        Serial.print("[SETUP] SSID: ");
        Serial.println(config.get().ssid);

        Serial.print("[SETUP] MQTT Host: ");
        Serial.println(config.get().mqttHost);

        Serial.print("[SETUP] MQTT Port: ");
        Serial.println(config.get().mqttPort);
    }
    else
    {
        Serial.println("[SETUP] No valid network configuration");
    }

    // --------------------------------------------------------
    // KEYPAD
    // --------------------------------------------------------

    Serial.println("[SETUP] 2. Keypad");

    keypad.begin();

    Serial.println("[SETUP] Keypad OK");

    // --------------------------------------------------------
    // LCD
    // --------------------------------------------------------

    Serial.println("[SETUP] 3. LCD");

    lcd.begin();

    Serial.println("[SETUP] LCD OK");

    // --------------------------------------------------------
    // RFID
    // --------------------------------------------------------

    Serial.println("[SETUP] 4. RFID");

    rfid.begin();

    Serial.println("[SETUP] RFID OK");

    // --------------------------------------------------------
    // INDICATOR
    // --------------------------------------------------------

    Serial.println("[SETUP] 5. Indicator");

    indicator.begin();

    Serial.println("[SETUP] Indicator OK");

    // --------------------------------------------------------
    // DOOR LOCK
    // --------------------------------------------------------

    Serial.println("[SETUP] 6. DoorLock");

    doorLock.begin();

    Serial.println("[SETUP] DoorLock OK");

    // --------------------------------------------------------
    // CONTROLLER
    // --------------------------------------------------------

    Serial.println("[SETUP] 7. LockController");

    controller.setState(
        &controller.idleState);

    Serial.println("[SETUP] LockController OK");

    // --------------------------------------------------------
    // MQTT SUBSCRIPTIONS
    // --------------------------------------------------------

    Serial.println("[SETUP] 8. MQTT subscriptions");

    mqtt.subscribe(
        AppConfig::Topics::OTP,
        [](const char *payload, unsigned int length)
        {
            otp.setOtp(payload, length);
            controller.onOtpReceived();
        });

    Serial.println("[SETUP] OTP subscription OK");

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

    Serial.println("[SETUP] Card verify subscription OK");

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

    Serial.println("[SETUP] Card enroll subscription OK");

    // --------------------------------------------------------
    // NO CONFIG -> WEB CONFIGURATION MODE
    // --------------------------------------------------------

    if (!hasConfig)
    {
        Serial.println();
        Serial.println("================================");
        Serial.println("     CONFIGURATION MODE");
        Serial.println("================================");

        Serial.println("[SETUP] Starting WebConfigService...");

        if (!webConfig.begin())
        {
            Serial.println("[SETUP] WebConfigService FAILED");
        }
        else
        {
            Serial.println("[SETUP] WebConfigService OK");

            Serial.println();
            Serial.println("Connect to Wi-Fi:");
            Serial.println("SSID: SmartLock-Setup");
            Serial.println("Open: http://192.168.4.1");
            Serial.println();
        }

        return;
    }

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial.println("[SETUP] 9. WiFi");

    if (!connectWifi())
    {
        Serial.println(
            "[SETUP] WiFi failed");

        Serial.println(
            "[SETUP] Starting WebConfigService...");

        webConfig.begin();

        return;
    }

    Serial.println("[SETUP] WiFi OK");

    // --------------------------------------------------------
    // MQTT
    // --------------------------------------------------------

    Serial.println("[SETUP] 10. MQTT begin");

    mqtt.begin();

    Serial.println("[SETUP] MQTT begin returned");

    // --------------------------------------------------------
    // SETUP FINISHED
    // --------------------------------------------------------

    Serial.println();
    Serial.println("================================");
    Serial.println("   [MAIN] Smart Lock started");
    Serial.println("================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    if (!config.hasConfig())
    {
        webConfig.handleClient();
        delay(2);

        return;
    }

    mqtt.loop();

    if (rfid.isCardPresent())
    {
        String uid = rfid.readUid();

        Serial.print("[RFID] UID: ");
        Serial.println(uid);

        controller.onCard(uid);
    }

    char key = keypad.readkey();
    if (key != NO_KEY)
    {
        controller.onKey(key);
    }
}
