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

    unsigned int n =
        length < AppConfig::Otp::LENGTH
            ? length
            : AppConfig::Otp::LENGTH;

    std::memcpy(currentOtp, otp, n);
    currentOtp[n] = '\0';

    otpSetAt = millis();
    attempts = 0;
    active = true;

    Serial.println("[OTP] New OTP received");
    Serial.println("[OTP] Attempt counter reset");
}

IOtpService::VerifyResult OtpService::verify(const char *input)
{
    if (!active || input == nullptr)
    {
        return VerifyResult::Inactive;
    }

    if (isExpired())
    {
        Serial.println("[OTP] OTP expired");
        clear();
        return VerifyResult::Expired;
    }

    if (attempts >= AppConfig::Otp::MAX_ATTEMPTS)
    {
        Serial.println("[OTP] Maximum invalid attempts exceeded");
        return VerifyResult::MaxAttemptsExceeded;
    }

    attempts++;

    Serial.print("[OTP] Verify attempt: ");
    Serial.println(attempts);

    if (std::strcmp(currentOtp, input) == 0)
    {
        Serial.println("[OTP] Valid");
        clear();
        return VerifyResult::Valid;
    }

    Serial.println("[OTP] Invalid");

    if (attempts >= AppConfig::Otp::MAX_ATTEMPTS)
    {
        Serial.println("[OTP] Maximum invalid attempts reached");
        return VerifyResult::MaxAttemptsExceeded;
    }

    return VerifyResult::Invalid;
}

bool OtpService::isExpired() const
{
    return active && (millis() - otpSetAt) >= AppConfig::Otp::TTL_MS;
}

bool OtpService::hasExceededMaxAttempts() const
{
    return attempts >= AppConfig::Otp::MAX_ATTEMPTS;
}

void OtpService::clear()
{
    currentOtp[0] = '\0';
    active = false;
    attempts = 0;
}
