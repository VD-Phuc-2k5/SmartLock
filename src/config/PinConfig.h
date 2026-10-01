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
        constexpr uint8_t MISO = 36;
        constexpr uint8_t MOSI = 38;
    }

    namespace Indicator
    {
        constexpr uint8_t GREEN_LED = 1;
        constexpr uint8_t RED_LED = 2;
        constexpr uint8_t BUZZER = 42;
    }
}
