#pragma once

#include "IKeypad.h"
#include "../config/AppConfig.h"
#include <Keypad.h>

class KeypadService : public IKeypad
{
private:
    Keypad keypad;

public:
    KeypadService();
    void begin() override;
    int readkey() override;
};
