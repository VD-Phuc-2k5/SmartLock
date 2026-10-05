#include <Arduino.h>
#include <cstring>
#include <WiFi.h>
#include <HTTPClient.h>

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
#include "camera/CameraService.h"

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
CameraService camera;

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

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(networkConfig.ssid.c_str(), networkConfig.password.c_str());

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
// CAMERA UPLOAD
// ============================================================

bool uploadCameraFrame(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0)
    {
        Serial.println("[CAMERA] Invalid frame");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[CAMERA] WiFi not connected");
        return false;
    }

    const NetworkConfig &networkConfig = config.get();

    String url =
        "http://" +
        networkConfig.mqttHost +
        ":" +
        String(AppConfig::Camera::SERVER_PORT) +
        AppConfig::Camera::SERVER_PATH;

    Serial.print("[CAMERA] POST ");
    Serial.println(url);

    WiFiClient client;
    HTTPClient http;

    http.setConnectTimeout(AppConfig::Camera::HTTP_TIMEOUT_MS);
    http.setTimeout(AppConfig::Camera::HTTP_TIMEOUT_MS);

    if (!http.begin(client, url))
    {
        Serial.println("[CAMERA] HTTP begin FAILED");
        return false;
    }

    http.addHeader("Content-Type", "image/jpeg");

    int httpCode = http.sendRequest("POST", const_cast<uint8_t *>(data), size);

    Serial.print("[CAMERA] HTTP status: ");
    Serial.println(httpCode);

    if (httpCode > 0)
    {
        String response = http.getString();
        Serial.print("[CAMERA] Server response: ");
        Serial.println(response);
    }
    else
    {
        Serial.print("[CAMERA] HTTP error: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end();

    return (
        httpCode >= 200 &&
        httpCode < 300
    );
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
    // CAMERA
    // --------------------------------------------------------

    Serial.println("[SETUP] 6. Camera");

    if (!camera.begin())
    {
        Serial.println("[SETUP] Camera FAILED");
    }
    else
    {
        Serial.println("[SETUP] Camera OK");

        CameraFrame frame = camera.capture();

        if (frame.data != nullptr && frame.size > 0)
        {
            Serial.print("[SETUP] Camera capture OK: ");
            Serial.print(frame.size);
            Serial.println(" bytes");
        }
        else
        {
            Serial.println("[SETUP] Camera capture FAILED");
        }

        camera.release();
    }

    // --------------------------------------------------------
    // DOOR LOCK
    // --------------------------------------------------------

    Serial.println("[SETUP] 7. DoorLock");

    doorLock.begin();

    Serial.println("[SETUP] DoorLock OK");

    // --------------------------------------------------------
    // CONTROLLER
    // --------------------------------------------------------

    Serial.println("[SETUP] 8. LockController");

    controller.setState(
        &controller.idleState);

    Serial.println("[SETUP] LockController OK");

    // --------------------------------------------------------
    // MQTT
    // --------------------------------------------------------

    Serial.println("[SETUP] 9. MQTT subscriptions");

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
    // WEB CONFIGURATION SERVER
    // --------------------------------------------------------

    Serial.println("[SETUP] Starting WebConfigService...");

    if (!webConfig.begin())
    {
        Serial.println("[SETUP] WebConfigService FAILED");
    }
    else
    {
        Serial.println("[SETUP] WebConfigService OK");
        Serial.println("[SETUP] Setup AP: SmartLock-Setup");
        Serial.println("[SETUP] Setup URL: http://192.168.4.1");
    }

    if (!hasConfig)
    {
        Serial.println("[SETUP] Waiting for network configuration");
        return;
    }

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    Serial.println("[SETUP] 10. WiFi");

    if (!connectWifi())
    {
        Serial.println("[SETUP] WiFi failed");
        return;
    }

    Serial.println("[SETUP] WiFi OK");

    // --------------------------------------------------------
    // MQTT BEGIN
    // --------------------------------------------------------

    Serial.println("[SETUP] 11. MQTT begin");

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
    webConfig.handleClient();

    if (!config.hasConfig())
    {
        delay(2);
        return;
    }

    mqtt.loop();
    // --------------------------------------------------------
    // RFID
    // --------------------------------------------------------

    if (rfid.isCardPresent())
    {
        String uid = rfid.readUid();

        Serial.print("[RFID] UID: ");
        Serial.println(uid);

        controller.onCard(uid);
    }

    // --------------------------------------------------------
    // KEYPAD
    // --------------------------------------------------------
    char key = keypad.readkey();
    if (key != NO_KEY)
    {
        Serial.print("[KEYPAD] Key: ");
        Serial.println(key);

        // ----------------------------------------------------
        // C = CAMERA CAPTURE
        // ----------------------------------------------------

        if (key == 'C')
        {
            Serial.println("[CAMERA] Capture requested");

            CameraFrame frame =camera.capture();

            if (frame.data != nullptr && frame.size > 0)
            {
                Serial.print("[CAMERA] Uploading ");
                Serial.print(frame.size);
                Serial.println(" bytes...");

                bool uploaded = uploadCameraFrame(frame.data, frame.size);

                if (uploaded)
                {
                    Serial.println("[CAMERA] Upload OK");
                }
                else
                {
                    Serial.println("[CAMERA] Upload FAILED");
                }
            }
            else
            {
                Serial.println("[CAMERA] Capture FAILED");
            }

            camera.release();
            return;
        }

        controller.onKey(key);
    }
}
