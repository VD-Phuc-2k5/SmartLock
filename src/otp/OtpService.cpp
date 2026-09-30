#include "OtpService.h"

#include <Arduino.h>
#include <cstring>

void OtpService::setOtp(const char *otp)
{
    if (otp == nullptr)
    {
        clear();
        return;
    }

    std::strncpy(currentOtp, otp, OTP_LENGTH);
    currentOtp[OTP_LENGTH] = '\0';

    otpSetAt = millis();
    attempts = 0;
    active = true;
}

bool OtpService::verify(const char *input)
{
    if (!active || input == nullptr)
    {
        return false;
    }

    if (isExpired() || attempts >= MAX_ATTEMPTS)
    {
        clear();
        return false;
    }

    attempts++;

    if (std::strcmp(currentOtp, input) == 0)
    {
        clear();
        return true;
    }

    return false;
}

bool OtpService::isExpired() const
{
    return active && (millis() - otpSetAt) >= OTP_TTL_MS;
}

void OtpService::clear()
{
    currentOtp[0] = '\0';
    active = false;
    attempts = 0;
}
