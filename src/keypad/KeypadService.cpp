#include "KeypadService.h"

namespace
{
    char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

    byte rowPins[4] = {4, 5, 6, 7};
    byte colPins[4] = {10, 11, 12, 13};
}

KeypadService::KeypadService() : keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS) {}

void KeypadService::begin()
{
}

int KeypadService::readkey()
{
    return keypad.getKey();
}