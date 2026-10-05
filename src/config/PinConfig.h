#pragma once

#include <cstdint>

namespace PinConfig
{
    namespace Keypad
    {
        constexpr uint8_t R1 = 41;
        constexpr uint8_t R2 = 42;
        constexpr uint8_t R3 = 45;
        constexpr uint8_t R4 = 0;

        constexpr uint8_t C1 = 48;
        constexpr uint8_t C2 = 47;
        constexpr uint8_t C3 = 20;
        constexpr uint8_t C4 = 19;
    }

    namespace Lcd
    {
        constexpr uint8_t SDA = 3;
        constexpr uint8_t SCL = 14;
    }

    namespace Rfid
    {
        constexpr uint8_t SS = 40;
        constexpr uint8_t RST = 0xFF;
        constexpr uint8_t SCK = 39;
        constexpr uint8_t MISO = 46;
        constexpr uint8_t MOSI = 38;
    }

    namespace Indicator
    {
        constexpr uint8_t GREEN_LED = 1;
        constexpr uint8_t RED_LED = 2;
        constexpr uint8_t BUZZER = 21;
    }

    namespace Lock
    {
        constexpr uint8_t SERVO = 0;
    }

    namespace Camera
    {
        constexpr int8_t PWDN = -1;
        constexpr int8_t RESET = -1;

        constexpr uint8_t XCLK = 15;
        constexpr uint8_t SIOD = 4;
        constexpr uint8_t SIOC = 5;
        constexpr uint8_t D0 = 11;
        constexpr uint8_t D1 = 9;
        constexpr uint8_t D2 = 8;
        constexpr uint8_t D3 = 10;
        constexpr uint8_t D4 = 12;
        constexpr uint8_t D5 = 18;
        constexpr uint8_t D6 = 17;
        constexpr uint8_t D7 = 16;
        constexpr uint8_t VSYNC = 6;
        constexpr uint8_t HREF = 7;
        constexpr uint8_t PCLK = 13;
    }
}