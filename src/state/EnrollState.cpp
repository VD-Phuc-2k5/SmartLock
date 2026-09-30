#include "EnrollState.h"
#include "LockController.h"
#include "../config/AppConfig.h"

void EnrollState::onEnter(LockController &ctx)
{
    ctx.lcd.clear();
    ctx.lcd.print("Scan card");
}

void EnrollState::onKey(LockController &ctx, char key)
{
    if (key == '#')
    {
        ctx.resetInput();
        ctx.setState(&ctx.idleState);
    }
}

void EnrollState::onCard(LockController &ctx, const String &uid)
{
    ctx.mqtt.publish(AppConfig::Topics::CARD_ENROLL, uid.c_str());
    ctx.lcd.clear();
    ctx.lcd.print("Enrolling ...");
}

void EnrollState::onVerifyResult(LockController &ctx, bool valid)
{
    (void)ctx;
    (void)valid;
}

void EnrollState::onEnrollResult(LockController &ctx, bool ok)
{
    ctx.lcd.clear();
    if (ok)
    {
        ctx.lcd.print("Enrolled");
    }
    else
    {
        ctx.lcd.print("Duplicate");
    }
}
