#include "OtpVerifyState.h"
#include "LockController.h"

void OtpVerifyState::onEnter(LockController &ctx)
{
    ctx.lcd.clear();
    ctx.lcd.print("Enter OTP");
}

void OtpVerifyState::onKey(LockController &ctx, char key)
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
        ctx.lcd.print("Enter OTP");
    }
    else if (key == '#')
    {
        if (ctx.otp.verify(ctx.getInput()))
        {
            ctx.resetInput();
            ctx.setState(&ctx.enrollState);
        }
        else
        {
            ctx.resetInput();
            ctx.lcd.clear();
            ctx.lcd.print("OTP INVALID");
            delay(1500);
            ctx.setState(&ctx.idleState);
        }
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
