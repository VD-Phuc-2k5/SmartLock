#include "LockController.h"

LockController::LockController(ILcd &lcd, IMqttClient &mqtt, IOtpService &otp)
    : lcd(lcd), mqtt(mqtt), otp(otp)
{
}

void LockController::setState(IState *state)
{
    current = state;
    if (current)
    {
        current->onEnter(*this);
    }
}

void LockController::onKey(char key)
{
    if (current)
    {
        current->onKey(*this, key);
    }
}

void LockController::onCard(const String &uid)
{
    if (current)
    {
        current->onCard(*this, uid);
    }
}

void LockController::onVerifyResult(bool valid)
{
    if (current)
    {
        current->onVerifyResult(*this, valid);
    }
}

void LockController::onEnrollResult(bool ok)
{
    if (current)
    {
        current->onEnrollResult(*this, ok);
    }
}

void LockController::resetInput()
{
    inputLength = 0;
    input[0] = '\0';
}

void LockController::appendKey(char key)
{
    if (inputLength < AppConfig::Otp::LENGTH)
    {
        input[inputLength++] = key;
        input[inputLength] = '\0';
    }
}

const char *LockController::getInput() const
{
    return input;
}
