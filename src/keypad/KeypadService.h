#pragma once

#include "IKeypad.h"
#include <Keypad.h>

class KeypadService : public IKeypad
{
private:
    static constexpr byte ROWS = 4;
    static constexpr byte COLS = 4;
    Keypad keypad;

public:
    KeypadService();
    void begin() override;
    int readkey() override;
};