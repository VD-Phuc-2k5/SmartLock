#pragma once

#include "IState.h"

class OtpVerifyState : public IState
{
public:
    void onEnter(LockController &ctx) override;
    void onKey(LockController &ctx, char key) override;
    void onCard(LockController &ctx, const String &uid) override;
    void onVerifyResult(LockController &ctx, bool valid) override;
    void onEnrollResult(LockController &ctx, bool ok) override;

    void setForAccess(bool value);

private:
    bool forAccess = false;
};
