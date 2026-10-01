#pragma once

#include <Arduino.h>
#include "IState.h"
#include "IdleState.h"
#include "OtpVerifyState.h"
#include "EnrollState.h"
#include "../lcd/ILcd.h"
#include "../mqtt/IMqttClient.h"
#include "../otp/IOtpService.h"
#include "../config/AppConfig.h"
#include "../indicator/AccessIndicator.h"
#include "../door/DoorLock.h"

class LockController
{
public:
    LockController(ILcd &lcd, IMqttClient &mqtt, IOtpService &otp, AccessIndicator &indicator, DoorLock &doorLock);

    void setState(IState *state);
    void onKey(char key);
    void onCard(const String &uid);
    void onVerifyResult(bool valid);
    void onEnrollResult(bool ok);
    void onOtpReceived();

    void resetInput();
    void appendKey(char key);
    const char *getInput() const;

    ILcd &lcd;
    IMqttClient &mqtt;
    IOtpService &otp;
    AccessIndicator &indicator;
    DoorLock &doorLock;

    IdleState idleState;
    OtpVerifyState otpVerifyState;
    EnrollState enrollState;

    bool verifyingForAccess = false;

private:
    IState *current = nullptr;
    char input[AppConfig::Otp::LENGTH + 1] = {};
    uint8_t inputLength = 0;
};
