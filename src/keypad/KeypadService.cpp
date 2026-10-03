#include "KeypadService.h"
#include "../config/AppConfig.h"
#include "../config/PinConfig.h"

namespace
{
    char keys[AppConfig::Keypad::ROWS][AppConfig::Keypad::COLS] = {
        {'A', 'B', 'C', 'D'},
        {'3', '6', '9', '#'},
        {'2', '5', '8', '0'},
        {'1', '4', '7', '*'}};

    byte rowPins[AppConfig::Keypad::ROWS] = {
        PinConfig::Keypad::R1,
        PinConfig::Keypad::R2,
        PinConfig::Keypad::R3,
        PinConfig::Keypad::R4,
    };

    byte colPins[AppConfig::Keypad::COLS] = {
        PinConfig::Keypad::C1,
        PinConfig::Keypad::C2,
        PinConfig::Keypad::C3,
        PinConfig::Keypad::C4,
    };
}

KeypadService::KeypadService()
    : keypad(makeKeymap(keys), rowPins, colPins, AppConfig::Keypad::ROWS, AppConfig::Keypad::COLS)
{
}

void KeypadService::begin()
{
}

int KeypadService::readkey()
{
    return keypad.getKey();
}
