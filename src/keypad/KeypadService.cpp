#include "KeypadService.h"
#include "../config/AppConfig.h"
#include "../config/PinConfig.h"

namespace
{
    char keys[AppConfig::Keypad::ROWS][AppConfig::Keypad::COLS] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

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
