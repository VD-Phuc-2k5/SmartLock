#include "IdleState.h"
#include "LockController.h"
#include "config/AppConfig.h"
#include <cstring>

void IdleState::onEnter(LockController &ctx)
{
    ctx.doorLock.lock();
    ctx.indicator.locked();
    ctx.lcd.clear();
    ctx.lcd.print("Waiting ...");
}

void IdleState::onKey(LockController &ctx, char key)
{
    if (key >= '0' && key <= '9')
    {
        ctx.appendKey(key);
        ctx.lcd.clear();
        ctx.lcd.print(ctx.getInput());
    }
    else if (key == '*')
    {
        ctx.resetInput();
        ctx.lcd.clear();
        ctx.lcd.print("Cleared");
    }
    else if (key == '#')
    {
        if (std::strcmp(ctx.getInput(), AppConfig::Password::MASTER) == 0)
        {
            ctx.resetInput();
            ctx.mqtt.publish(AppConfig::Topics::OTP_REQUEST, "1");
            ctx.setState(&ctx.otpVerifyState);
        }
        else
        {
            ctx.resetInput();
            ctx.lcd.clear();
            ctx.lcd.print("Wrong password");
        }
    }
}

void IdleState::onCard(LockController &ctx, const String &uid)
{
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
    }
    else
    {
        ctx.lcd.print("No info");
        ctx.indicator.accessDenied();
        delay(AppConfig::FAILURE_DELAY_MS);
        ctx.lcd.clear();
        ctx.lcd.print("Waiting ...");
    }
}

void IdleState::onEnrollResult(LockController &ctx, bool ok)
{
    (void)ctx;
    (void)ok;
}
