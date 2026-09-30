#pragma once

class IOtpService
{
public:
    virtual ~IOtpService() = default;

    virtual void setOtp(const char *otp) = 0;
    virtual bool verify(const char *input) = 0;
    virtual bool isExpired() const = 0;
    virtual void clear() = 0;
};
