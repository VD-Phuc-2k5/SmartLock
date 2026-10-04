#include "OtpVerifyState.h"
#include "LockController.h"
#include "../config/AppConfig.h"

void OtpVerifyState::setForAccess(bool value)
{
    forAccess = value;
}

void OtpVerifyState::onEnter(LockController &ctx)
{
    waitingForDecision = false;
    ctx.resetInput();
    ctx.lcd.clear();
    ctx.lcd.print("Enter OTP");
}

void OtpVerifyState::onKey(LockController &ctx, char key)
{
    if (waitingForDecision)
    {
        if (key == 'A')
        {
            waitingForDecision = false;
            ctx.requestNewOtp();
            return;
        }

        if (key == 'D')
        {
            waitingForDecision = false;
            forAccess = false;

            ctx.otp.clear();
            ctx.resetInput();
            ctx.verifyingForAccess = false;

            Serial.println("[OTP] User selected D");
            Serial.println("[OTP] Access denied");

            ctx.lcd.clear();
            ctx.lcd.print("Access denied");

            ctx.indicator.accessDenied();

            delay(AppConfig::Otp::FAILURE_DELAY_MS);

            ctx.setState(&ctx.idleState);
            return;
        }

        return;
    }

    if (key >= '0' && key <= '9')
    {
        ctx.appendKey(key);
        ctx.lcd.clear();
        ctx.lcd.print(ctx.getInput());
        return;
    }

    if (key == '*')
    {
        ctx.resetInput();
        ctx.lcd.clear();
        ctx.lcd.print("Enter OTP");
        return;
    }

    if (key != '#')
    {
        return;
    }

    const auto result = ctx.otp.verify(ctx.getInput());

    ctx.resetInput();

    switch (result)
    {
    case IOtpService::VerifyResult::Valid:
        if (forAccess)
        {
            forAccess = false;
            ctx.verifyingForAccess = false;

            ctx.lcd.clear();
            ctx.lcd.print("Access granted");

            ctx.indicator.unlocked();
            ctx.doorLock.unlock();

            delay(AppConfig::Otp::DELAY_MS);

            ctx.setState(&ctx.idleState);
        }
        else
        {
            ctx.setState(&ctx.enrollState);
        }
        break;

    case IOtpService::VerifyResult::Invalid:
        ctx.lcd.clear();
        ctx.lcd.print("OTP INVALID");

        ctx.indicator.accessDenied();

        delay(AppConfig::Otp::FAILURE_DELAY_MS);

        ctx.lcd.clear();
        ctx.lcd.print("Enter OTP");
        break;

    case IOtpService::VerifyResult::MaxAttemptsExceeded:
        waitingForDecision = true;

        Serial.println("[OTP] Maximum invalid attempts reached");
        Serial.println("[OTP] Press A to request new OTP");
        Serial.println("[OTP] Press D to deny access");

        ctx.indicator.accessDenied();

        ctx.lcd.clear();
        ctx.lcd.print("Max attempts");

        delay(AppConfig::Otp::FAILURE_DELAY_MS);

        ctx.lcd.clear();
        ctx.lcd.print("A:New OTP D:Deny");
        break;

    case IOtpService::VerifyResult::Expired:
        Serial.println("[OTP] OTP expired");

        ctx.otp.clear();
        ctx.verifyingForAccess = false;
        forAccess = false;

        ctx.lcd.clear();
        ctx.lcd.print("OTP expired");

        delay(AppConfig::Otp::FAILURE_DELAY_MS);

        ctx.setState(&ctx.idleState);
        break;

    case IOtpService::VerifyResult::Inactive:
        Serial.println("[OTP] OTP inactive");

        ctx.otp.clear();
        ctx.verifyingForAccess = false;
        forAccess = false;

        ctx.lcd.clear();
        ctx.lcd.print("OTP unavailable");

        delay(AppConfig::Otp::FAILURE_DELAY_MS);

        ctx.setState(&ctx.idleState);
        break;
    }
}

void OtpVerifyState::onCard(LockController &ctx, const String &uid)
{
    (void)ctx;
    (void)uid;
}

void OtpVerifyState::onVerifyResult(LockController &ctx, bool valid)
{
    (void)ctx;
    (void)valid;
}

void OtpVerifyState::onEnrollResult(LockController &ctx, bool ok)
{
    (void)ctx;
    (void)ok;
}

void OtpVerifyState::onOtpReceived(LockController &ctx)
{
    waitingForDecision = false;
    forAccess = true;

    ctx.resetInput();

    Serial.println("[OTP] New OTP received");
    Serial.println("[OTP] Attempt counter reset");
    Serial.println("[OTP] Enter new OTP");

    ctx.lcd.clear();
    ctx.lcd.print("Enter new OTP");
}
