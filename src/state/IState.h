#pragma once

#include <Arduino.h>

class LockController;

class IState
{
public:
    virtual ~IState() = default;

    virtual void onEnter(LockController &ctx) = 0;
    virtual void onKey(LockController &ctx, char key) = 0;
    virtual void onCard(LockController &ctx, const String &uid) = 0;
    virtual void onVerifyResult(LockController &ctx, bool valid) = 0;
    virtual void onEnrollResult(LockController &ctx, bool ok) = 0;
};
