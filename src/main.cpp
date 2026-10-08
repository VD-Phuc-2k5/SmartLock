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

static volatile uint8_t g_lastDisconnectReason = 0;

static void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("[WIFI][EVT] Associated with AP");
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        g_lastDisconnectReason = info.wifi_sta_disconnected.reason;
        Serial.printf(
            "[WIFI][EVT] Disconnected, reason=%u\n",
            g_lastDisconnectReason);
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.println("[WIFI][EVT] Got IP");
        break;

    default:
        break;
    }
}

static const char *authModeName(wifi_auth_mode_t m)
{
    switch (m)
    {
    case WIFI_AUTH_OPEN:            return "OPEN";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2-PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENTERPRISE (802.1X)";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3-PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3-PSK";
    default:                        return "OTHER";
    }
}

static const char *disconnectReasonName(uint8_t r)
{
    switch (r)
    {
    case 2:   return "AUTH_EXPIRE";
    case 8:   return "ASSOC_LEAVE (tu ngat)";
    case 15:  return "4WAY_HANDSHAKE_TIMEOUT (sai mat khau)";
    case 201: return "NO_AP_FOUND";
    case 202: return "AUTH_FAIL (sai mat khau / sai kieu bao mat)";
    case 203: return "ASSOC_FAIL";
    case 204: return "HANDSHAKE_TIMEOUT (sai mat khau)";
    case 205: return "CONNECTION_FAIL";
    default:  return "OTHER";
    }
}

bool connectWifi()
{
    const NetworkConfig &networkConfig = config.get();

    Serial.println("[WIFI] Connecting...");
    Serial.print("[WIFI] SSID: ");
    Serial.println(networkConfig.ssid);
    Serial.print("[WIFI] Password length: ");
    Serial.println(networkConfig.password.length());

    // Reset sach trang thai WiFi (tranh con sot tu che do AP / lan thu truoc)
    WiFi.onEvent(onWifiEvent);
    WiFi.persistent(false);
    WiFi.disconnect(true, true);
    delay(200);
    WiFi.mode(WIFI_OFF);
    delay(200);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(false);
    delay(100);

    // ---- Scan: tim AP manh nhat co dung SSID, lay BSSID + channel + authmode
    Serial.println("[WIFI] Scanning networks...");
    const int found = WiFi.scanNetworks();
    Serial.printf("[WIFI] Found %d networks\n", found);

    int bestIdx = -1;
    int bestRssi = -1000;

    for (int i = 0; i < found; i++)
    {
        Serial.printf(
            "[SCAN] %s | RSSI: %d | CH: %d | %s\n",
            WiFi.SSID(i).c_str(),
            WiFi.RSSI(i),
            WiFi.channel(i),
            authModeName(WiFi.encryptionType(i)));

        if (WiFi.SSID(i) == networkConfig.ssid &&
            WiFi.RSSI(i) > bestRssi)
        {
            bestRssi = WiFi.RSSI(i);
            bestIdx = i;
        }
    }

    uint8_t bssid[6] = {0};
    int32_t channel = 0;
    bool haveTarget = false;

    if (bestIdx >= 0)
    {
        const wifi_auth_mode_t auth = WiFi.encryptionType(bestIdx);
        memcpy(bssid, WiFi.BSSID(bestIdx), 6);
        channel = WiFi.channel(bestIdx);
        haveTarget = true;

        Serial.printf(
            "[WIFI] Target AP: %02X:%02X:%02X:%02X:%02X:%02X CH %d RSSI %d Auth: %s\n",
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
            (int)channel, bestRssi, authModeName(auth));

        if (auth == WIFI_AUTH_WPA2_ENTERPRISE)
        {
            Serial.println("[WIFI] ERROR: mang dung 802.1X (user/pass dang nhap), "
                           "WiFi.begin(ssid, pass) KHONG ho tro. "
                           "Hay dung WiFi khac (hotspot dien thoai) hoac code WPA2-Enterprise.");
            WiFi.scanDelete();
            return false;
        }

        if (auth != WIFI_AUTH_OPEN && networkConfig.password.length() == 0)
        {
            Serial.println("[WIFI] ERROR: mang co mat khau nhung password dang TRONG. "
                           "Vao trang cau hinh (SmartLock-Setup) nhap lai password.");
            WiFi.scanDelete();
            return false;
        }

        if (auth == WIFI_AUTH_OPEN && networkConfig.password.length() > 0)
        {
            Serial.println("[WIFI] WARN: mang OPEN nhung co password -> bo qua password");
        }
    }
    else
    {
        Serial.println("[WIFI] WARN: khong thay SSID trong ket qua scan "
                       "(sai ten SSID? SSID co dau cach/ky tu thua?)");
    }

    WiFi.scanDelete();

    // ---- Thu ket noi: lan 1 khoa BSSID/channel, lan 2-3 de tu do
    const char *pass =
        networkConfig.password.length() > 0
            ? networkConfig.password.c_str()
            : nullptr;

    constexpr int MAX_ATTEMPTS = 3;
    constexpr uint32_t ATTEMPT_TIMEOUT_MS = 15000;

    // De driver tu retry noi bo trong luc cho (mang open/captive can vai giay)
    WiFi.setAutoReconnect(true);

    for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++)
    {
        Serial.printf("[WIFI] Attempt %d/%d\n", attempt, MAX_ATTEMPTS);

        if (attempt > 1)
        {
            // Chi ngat khi lan truoc da that bai hoan toan
            WiFi.disconnect(false, false);
            delay(500);
        }

        g_lastDisconnectReason = 0;

        if (attempt == 1 && haveTarget)
        {
            WiFi.begin(networkConfig.ssid.c_str(), pass, channel, bssid, true);
        }
        else
        {
            WiFi.begin(networkConfig.ssid.c_str(), pass);
        }

        const unsigned long start = millis();

        // KHONG thoat som khi gap disconnect reason: driver co the tu retry.
        while (WiFi.status() != WL_CONNECTED &&
               millis() - start < ATTEMPT_TIMEOUT_MS)
        {
            delay(250);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            break;
        }

        Serial.printf(
            "[WIFI] Attempt %d timeout, status=%d, last reason=%u (%s)\n",
            attempt,
            (int)WiFi.status(),
            g_lastDisconnectReason,
            disconnectReasonName(g_lastDisconnectReason));
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.printf(
            "[WIFI] Connection FAILED, status=%d, reason=%u (%s)\n",
            (int)WiFi.status(),
            g_lastDisconnectReason,
            disconnectReasonName(g_lastDisconnectReason));
        return false;
    }

    Serial.println("[WIFI] Connected");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WIFI] RSSI: ");
    Serial.println(WiFi.RSSI());

    return true;
}

// ============================================================
// CAMERA UPLOAD
// ============================================================

bool uploadCameraFrame(
    const uint8_t *data,
    size_t size)
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

    http.setConnectTimeout(
        AppConfig::Camera::HTTP_TIMEOUT_MS);

    http.setTimeout(
        AppConfig::Camera::HTTP_TIMEOUT_MS);

    if (!http.begin(client, url))
    {
        Serial.println("[CAMERA] HTTP begin FAILED");
        return false;
    }

    http.addHeader(
        "Content-Type",
        "image/jpeg");

    const int httpCode =
        http.sendRequest(
            "POST",
            const_cast<uint8_t *>(data),
            size);

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
        Serial.println(
            http.errorToString(httpCode));
    }

    http.end();

    return (
        httpCode >= 200 &&
        httpCode < 300);
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
    // 1. CONFIG
    // --------------------------------------------------------

    Serial.println("[SETUP] 1. ConfigManager");

    const bool hasConfig = config.begin();

    if (hasConfig)
    {
        Serial.println(
            "[SETUP] Network configuration loaded");

        Serial.print("[SETUP] SSID: ");
        Serial.println(config.get().ssid);

        Serial.print("[SETUP] MQTT Host: ");
        Serial.println(config.get().mqttHost);

        Serial.print("[SETUP] MQTT Port: ");
        Serial.println(config.get().mqttPort);
    }
    else
    {
        Serial.println(
            "[SETUP] No valid network configuration");
    }

    // --------------------------------------------------------
    // 2. KEYPAD
    // --------------------------------------------------------

    Serial.println("[SETUP] 2. Keypad");

    keypad.begin();

    Serial.println("[SETUP] Keypad OK");

    // --------------------------------------------------------
    // 3. LCD
    // --------------------------------------------------------

    Serial.println("[SETUP] 3. LCD");

    lcd.begin();

    Serial.println("[SETUP] LCD OK");

    // --------------------------------------------------------
    // 4. RFID
    // --------------------------------------------------------

    Serial.println("[SETUP] 4. RFID");

    rfid.begin();

    Serial.println("[SETUP] RFID OK");

    // --------------------------------------------------------
    // 5. INDICATOR
    // --------------------------------------------------------

    Serial.println("[SETUP] 5. Indicator");

    indicator.begin();

    Serial.println("[SETUP] Indicator OK");

    // --------------------------------------------------------
    // 6. DOOR LOCK
    // --------------------------------------------------------

    Serial.println("[SETUP] 6. DoorLock");

    doorLock.begin();

    Serial.println("[SETUP] DoorLock OK");

    // --------------------------------------------------------
    // 7. LOCK CONTROLLER
    // --------------------------------------------------------

    Serial.println("[SETUP] 7. LockController");

    controller.setState(
        &controller.idleState);

    Serial.println("[SETUP] LockController OK");

    // --------------------------------------------------------
    // 8. MQTT SUBSCRIPTIONS
    // --------------------------------------------------------

    Serial.println(
        "[SETUP] 8. MQTT subscriptions");

    mqtt.subscribe(
        AppConfig::Topics::OTP,
        [](const char *payload,
           unsigned int length)
        {
            otp.setOtp(
                payload,
                length);

            controller.onOtpReceived();
        });

    Serial.println(
        "[SETUP] OTP subscription OK");

    mqtt.subscribe(
        AppConfig::Topics::CARD_VERIFY_RESULT,
        [](const char *payload,
           unsigned int length)
        {
            const bool valid =
                length == 5 &&
                std::strncmp(
                    payload,
                    "valid",
                    5) == 0;

            controller.onVerifyResult(valid);
        });

    Serial.println(
        "[SETUP] Card verify subscription OK");

    mqtt.subscribe(
        AppConfig::Topics::CARD_ENROLL_RESULT,
        [](const char *payload,
           unsigned int length)
        {
            const bool ok =
                length == 2 &&
                std::strncmp(
                    payload,
                    "ok",
                    2) == 0;

            controller.onEnrollResult(ok);
        });

    Serial.println(
        "[SETUP] Card enroll subscription OK");

    // --------------------------------------------------------
    // 9. WEB CONFIGURATION
    // --------------------------------------------------------

    if (!hasConfig)
    {
        Serial.println(
            "[SETUP] Starting WebConfigService...");

        if (!webConfig.begin())
        {
            Serial.println(
                "[SETUP] WebConfigService FAILED");
        }
        else
        {
            Serial.println(
                "[SETUP] WebConfigService OK");

            Serial.println(
                "[SETUP] Setup AP: SmartLock-Setup");

            Serial.println(
                "[SETUP] Setup URL: http://192.168.4.1");
        }

        Serial.println(
            "[SETUP] Waiting for network configuration");

        return;
    }

    Serial.println(
        "[SETUP] Starting WebConfigService...");

    if (!webConfig.begin())
    {
        Serial.println(
            "[SETUP] WebConfigService FAILED");
    }
    else
    {
        Serial.println(
            "[SETUP] WebConfigService OK");
    }

    // --------------------------------------------------------
    // 10. WIFI
    //
    // IMPORTANT:
    // Camera is NOT initialized before WiFi.
    // --------------------------------------------------------

    Serial.println("[SETUP] 10. WiFi");

    if (!connectWifi())
    {
        Serial.println(
            "[SETUP] WiFi FAILED");

        return;
    }

    Serial.println("[SETUP] WiFi OK");

    // --------------------------------------------------------
    // 11. CAMERA
    //
    // Initialize camera only after WiFi is completely ready.
    // --------------------------------------------------------

    Serial.println("[SETUP] 11. Camera");

    if (!camera.begin())
    {
        Serial.println("[SETUP] Camera FAILED");
    }
    else
    {
        Serial.println("[SETUP] Camera OK");

        Serial.print("[CAMERA] PSRAM: ");
        Serial.println(
            psramFound()
                ? "YES"
                : "NO");

        Serial.print("[CAMERA] Free PSRAM: ");
        Serial.println(
            ESP.getFreePsram());

        Serial.print("[CAMERA] Free Heap: ");
        Serial.println(
            ESP.getFreeHeap());
    }

    // --------------------------------------------------------
    // 12. MQTT
    // --------------------------------------------------------

    Serial.println("[SETUP] 12. MQTT begin");

    mqtt.begin();

    Serial.println(
        "[SETUP] MQTT begin returned");

    // --------------------------------------------------------
    // DONE
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "================================");
    Serial.println(
        "   SMART LOCK READY");
    Serial.println(
        "================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    webConfig.handleClient();

    if (!config.hasConfig())
    {
        delay(10);
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

    const char key = keypad.readkey();

    if (key != NO_KEY)
    {
        Serial.print("[KEYPAD] Key: ");
        Serial.println(key);

        if (key == 'C')
        {
            Serial.println(
                "[CAMERA] Capture requested");

            CameraFrame frame =
                camera.capture();

            if (
                frame.data != nullptr &&
                frame.size > 0)
            {
                Serial.print(
                    "[CAMERA] Uploading ");

                Serial.print(frame.size);

                Serial.println(" bytes...");

                const bool uploaded =
                    uploadCameraFrame(
                        frame.data,
                        frame.size);

                if (uploaded)
                {
                    Serial.println(
                        "[CAMERA] Upload OK");
                }
                else
                {
                    Serial.println(
                        "[CAMERA] Upload FAILED");
                }
            }
            else
            {
                Serial.println(
                    "[CAMERA] Capture FAILED");
            }

            camera.release();
        }
        else
        {
            controller.onKey(key);
        }
    }

    delay(2);
}