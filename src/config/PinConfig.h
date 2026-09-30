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
}