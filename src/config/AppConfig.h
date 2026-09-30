#pragma once

#include <cstdint>

namespace AppConfig
{
    namespace Wifi
    {
        constexpr char SSID[] = "Wokwi-GUEST";
        constexpr char PASSWORD[] = "";
        constexpr uint32_t TIMEOUT_MS = 15000;
    }

    namespace Mqtt
    {
        constexpr char BROKER[] = "host.wokwi.internal";
        constexpr uint16_t PORT = 1883;
        constexpr char CLIENT_ID[] = "smartlock-esp32-01";
        constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
        constexpr uint8_t MAX_SUBSCRIPTIONS = 4;
    }

    namespace Otp
    {
        constexpr uint8_t LENGTH = 6;
        constexpr uint32_t TTL_MS = 300000;
        constexpr uint8_t MAX_ATTEMPTS = 3;
    }

    namespace Password
    {
        constexpr char MASTER[] = "111111";
        constexpr uint8_t LENGTH = 6;
    }

    namespace Lcd
    {
        constexpr uint8_t ADDRESS = 0x27;
        constexpr uint8_t COLUMNS = 16;
        constexpr uint8_t ROWS = 2;
    }

    namespace Keypad
    {
        constexpr uint8_t ROWS = 4;
        constexpr uint8_t COLS = 4;
    }

    namespace Topics
    {
        constexpr char OTP[] = "smartlock/device/esp32-01/otp";
        constexpr char OTP_REQUEST[] = "smartlock/device/esp32-01/otp/request";
        constexpr char CARD_ENROLL[] = "smartlock/device/esp32-01/card/enroll";
        constexpr char CARD_VERIFY[] = "smartlock/device/esp32-01/card/verify";
        constexpr char CARD_ENROLL_RESULT[] = "smartlock/device/esp32-01/card/enroll/result";
        constexpr char CARD_VERIFY_RESULT[] = "smartlock/device/esp32-01/card/verify/result";
    }
}
