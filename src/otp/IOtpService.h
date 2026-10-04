#pragma once

class IOtpService
{
public:
    enum class VerifyResult
    {
        Valid,
        Invalid,
        MaxAttemptsExceeded,
        Expired,
        Inactive
    };

    virtual ~IOtpService() = default;

    virtual void setOtp(const char *otp, unsigned int length) = 0;
    virtual VerifyResult verify(const char *input) = 0;
    virtual bool isExpired() const = 0;
    virtual bool hasExceededMaxAttempts() const = 0;
    virtual void clear() = 0;
};
