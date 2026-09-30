#pragma once

class IOtpService
{
public:
    virtual ~IOtpService() = default;

    virtual void setOtp(const char *otp, unsigned int length) = 0;
    virtual bool verify(const char *input) = 0;
    virtual bool isExpired() const = 0;
    virtual void clear() = 0;
};
