#pragma once

#include "IOtpService.h"
#include "../config/AppConfig.h"
#include <cstdint>

class OtpService : public IOtpService
{
private:
    char currentOtp[AppConfig::Otp::LENGTH + 1] = {};
    uint32_t otpSetAt = 0;
    uint8_t attempts = 0;
    bool active = false;

public:
    void setOtp(const char *otp, unsigned int length) override;
    VerifyResult verify(const char *input) override;
    bool isExpired() const override;
    bool hasExceededMaxAttempts() const override;
    void clear() override;
};
