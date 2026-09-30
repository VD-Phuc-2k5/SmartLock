#include "OtpService.h"

#include <Arduino.h>
#include <cstring>

void OtpService::setOtp(const char *otp, unsigned int length)
{
    if (otp == nullptr || length == 0)
    {
        clear();
        return;
    }

    unsigned int n = length < AppConfig::Otp::LENGTH ? length : AppConfig::Otp::LENGTH;
    std::memcpy(currentOtp, otp, n);
    currentOtp[n] = '\0';

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

    if (isExpired() || attempts >= AppConfig::Otp::MAX_ATTEMPTS)
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
    return active && (millis() - otpSetAt) >= AppConfig::Otp::TTL_MS;
}

void OtpService::clear()
{
    currentOtp[0] = '\0';
    active = false;
    attempts = 0;
}
