#pragma once

#include "IOtpService.h"
#include <cstdint>

class OtpService : public IOtpService
{
private:
    static constexpr uint8_t OTP_LENGTH = 6;
    static constexpr uint32_t OTP_TTL_MS = 300000;
    static constexpr uint8_t MAX_ATTEMPTS = 3;

    char currentOtp[OTP_LENGTH + 1] = {};
    uint32_t otpSetAt = 0;
    uint8_t attempts = 0;
    bool active = false;

public:
    void setOtp(const char *otp) override;
    bool verify(const char *input) override;
    bool isExpired() const override;
    void clear() override;
};
