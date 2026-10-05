#pragma once

#include <cstddef>
#include <cstdint>

struct CameraFrame
{
    const uint8_t *data = nullptr;
    size_t size = 0;
};

class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual bool begin() = 0;
    virtual CameraFrame capture() = 0;
    virtual void release() = 0;
};