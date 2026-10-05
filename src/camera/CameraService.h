#pragma once

#include "ICamera.h"
#include <esp_camera.h>

class CameraService : public ICamera
{
public:
    bool begin() override;
    CameraFrame capture() override;
    void release() override;

private:
    bool initialized = false;
    camera_fb_t *currentFrame = nullptr;
};