#pragma once

#include <cstdint>

namespace PinConfig
{
    namespace Keypad
    {
        constexpr uint8_t R1 = 4;
        constexpr uint8_t R2 = 5;
        constexpr uint8_t R3 = 6;
        constexpr uint8_t R4 = 7;

        constexpr uint8_t C1 = 10;
        constexpr uint8_t C2 = 11;
        constexpr uint8_t C3 = 12;
        constexpr uint8_t C4 = 13;
    }

    namespace Lcd
    {
        constexpr uint8_t SDA = 8;
        constexpr uint8_t SCL = 9;
    }

    namespace Rfid
    {
        constexpr uint8_t SS = 40;
        constexpr uint8_t RST = 35;
        constexpr uint8_t SCK = 39;
        constexpr uint8_t MISO = 37;
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
        constexpr uint8_t SERVO = 15;
    }

    namespace Camera
    {
        constexpr int8_t PWDN = -1;
        constexpr int8_t RESET = -1;

        constexpr uint8_t XCLK = 42;

        constexpr uint8_t SIOD = 43;
        constexpr uint8_t SIOC = 44;

        constexpr uint8_t D0 = 14;
        constexpr uint8_t D1 = 15;
        constexpr uint8_t D2 = 16;
        constexpr uint8_t D3 = 17;
        constexpr uint8_t D4 = 18;
        constexpr uint8_t D5 = 19;
        constexpr uint8_t D6 = 20;
        constexpr uint8_t D7 = 41;

        constexpr uint8_t VSYNC = 47;
        constexpr uint8_t HREF = 48;
        constexpr uint8_t PCLK = 45;
    }
}