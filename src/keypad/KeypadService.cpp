#include "KeypadService.h"
#include "../config/PinConfig.h"

namespace
{
    char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

    byte rowPins[4] = {
        PinConfig::Keypad::R1,
        PinConfig::Keypad::R2,
        PinConfig::Keypad::R3,
        PinConfig::Keypad::R4,
    };

    byte colPins[4] = {
        PinConfig::Keypad::C1,
        PinConfig::Keypad::C2,
        PinConfig::Keypad::C3,
        PinConfig::Keypad::C4,
    };
}

KeypadService::KeypadService() : keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS) {}

void KeypadService::begin()
{
}

int KeypadService::readkey()
{
    return keypad.getKey();
}