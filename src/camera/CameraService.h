#pragma once

#include "ICamera.h"

class CameraService : public ICamera
{
public:
    bool begin() override;
    CameraFrame capture() override;
    void release() override;

private:
    bool initialized = false;
};