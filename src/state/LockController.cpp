#include "LockController.h"

LockController::LockController(ILcd &lcd, IMqttClient &mqtt, IOtpService &otp, AccessIndicator &indicator, DoorLock &doorLock)
    : lcd(lcd), mqtt(mqtt), otp(otp), indicator(indicator), doorLock(doorLock)
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

void LockController::onOtpReceived()
{
    if (waitingForNewOtp)
    {
        waitingForNewOtp = false;

        resetInput();

        Serial.println("[OTP] New OTP received");
        Serial.println("[OTP] Attempt counter reset");

        if (current == &otpVerifyState)
        {
            otpVerifyState.setForAccess(true);

            lcd.clear();
            lcd.print("Enter new OTP");
        }

        return;
    }

    if (verifyingForAccess && current == &idleState)
    {
        verifyingForAccess = false;

        resetInput();

        otpVerifyState.setForAccess(true);
        setState(&otpVerifyState);

        Serial.println("[OTP] Access OTP received");
        Serial.println("[OTP] Waiting for user input");

        return;
    }
}

void LockController::requestNewOtp()
{
    Serial.println("[OTP] User selected A");
    Serial.println("[OTP] Requesting new OTP...");

    resetInput();

    waitingForNewOtp = true;

    mqtt.publish(
        AppConfig::Topics::OTP_REQUEST,
        "1");

    lcd.clear();
    lcd.print("Requesting OTP");

    Serial.println("[OTP] Waiting for backend OTP...");
}

void LockController::resetInput()
{
    inputLength = 0;
    input[0] = '\0';
}

void LockController::appendKey(char key)
{
    if (inputLength >= AppConfig::Otp::LENGTH)
    {
        return;
    }

    input[inputLength++] = key;
    input[inputLength] = '\0';
}

const char *LockController::getInput() const
{
    return input;
}
