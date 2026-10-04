#include "IdleState.h"
#include "LockController.h"
#include "../config/AppConfig.h"
#include <cstring>

void IdleState::onEnter(LockController &ctx)
{
    ctx.verifyingForAccess = false;
    ctx.otp.clear();
    ctx.resetInput();
    ctx.doorLock.lock();
    ctx.indicator.locked();
    ctx.lcd.clear();
    ctx.lcd.print("Waiting ...");
    Serial.println("[IDLE] Ready for RFID");
}

void IdleState::onKey(LockController &ctx, char key)
{
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
        ctx.lcd.print("Cleared");
        return;
    }

    if (key == '#')
    {
        if (std::strcmp(ctx.getInput(), AppConfig::Password::MASTER) == 0)
        {
            ctx.resetInput();
            ctx.verifyingForAccess = false;
            ctx.mqtt.publish(AppConfig::Topics::OTP_REQUEST, "1");
            ctx.setState(&ctx.otpVerifyState);
            return;
        }

        ctx.resetInput();

        ctx.lcd.clear();
        ctx.lcd.print("Wrong password");
    }
}

void IdleState::onCard(LockController &ctx, const String &uid)
{
    Serial.print("[RFID] Card detected: ");
    Serial.println(uid);
    ctx.mqtt.publish(AppConfig::Topics::CARD_VERIFY, uid.c_str());
    ctx.lcd.clear();
    ctx.lcd.print("Checking ...");
}

void IdleState::onVerifyResult(LockController &ctx, bool valid)
{
    ctx.lcd.clear();
    if (valid)
    {
        ctx.verifyingForAccess = true;
        ctx.lcd.print("Waiting OTP...");

        Serial.println("[RFID] Card valid");
        Serial.println("[RFID] Waiting for access OTP");

        return;
    }

    ctx.verifyingForAccess = false;
    ctx.lcd.print("No info");
    ctx.indicator.accessDenied();

    delay(AppConfig::Otp::FAILURE_DELAY_MS);

    ctx.lcd.clear();
    ctx.lcd.print("Waiting ...");

    Serial.println("[RFID] Card denied");
}

void IdleState::onEnrollResult(LockController &ctx, bool ok)
{
    (void)ctx;
    (void)ok;
}

void IdleState::onOtpReceived(LockController &ctx)
{
    if (!ctx.verifyingForAccess)
    {
        return;
    }

    ctx.verifyingForAccess = false;
    ctx.resetInput();
    ctx.otpVerifyState.setForAccess(true);

    Serial.println("[OTP] Access OTP received");
    Serial.println("[OTP] Waiting for user input");

    ctx.setState(&ctx.otpVerifyState);
}
