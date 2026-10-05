#include "CameraService.h"

#include "../config/PinConfig.h"

#include <Arduino.h>
#include <esp_camera.h>

bool CameraService::begin()
{
    camera_config_t config{};

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    config.pin_d0 = PinConfig::Camera::D0;
    config.pin_d1 = PinConfig::Camera::D1;
    config.pin_d2 = PinConfig::Camera::D2;
    config.pin_d3 = PinConfig::Camera::D3;
    config.pin_d4 = PinConfig::Camera::D4;
    config.pin_d5 = PinConfig::Camera::D5;
    config.pin_d6 = PinConfig::Camera::D6;
    config.pin_d7 = PinConfig::Camera::D7;

    config.pin_xclk = PinConfig::Camera::XCLK;
    config.pin_pclk = PinConfig::Camera::PCLK;
    config.pin_vsync = PinConfig::Camera::VSYNC;
    config.pin_href = PinConfig::Camera::HREF;

    config.pin_sccb_sda = PinConfig::Camera::SIOD;
    config.pin_sccb_scl = PinConfig::Camera::SIOC;

    config.pin_pwdn = PinConfig::Camera::PWDN;
    config.pin_reset = PinConfig::Camera::RESET;

    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound())
    {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 8;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }

    Serial.println("[CAMERA] Initializing OV2640...");

    esp_err_t result = esp_camera_init(&config);

    if (result != ESP_OK)
    {
        Serial.print("[CAMERA] Init failed: 0x");
        Serial.println(result, HEX);

        initialized = false;
        return false;
    }

    sensor_t *sensor = esp_camera_sensor_get();

    if (sensor != nullptr)
    {
        sensor->set_framesize(
            sensor,
            psramFound()
                ? FRAMESIZE_QVGA
                : FRAMESIZE_QQVGA);

        sensor->set_quality(
            sensor,
            psramFound()
                ? 12
                : 15);
    }

    initialized = true;

    Serial.println("[CAMERA] OV2640 initialized");

    Serial.print("[CAMERA] PSRAM: ");
    Serial.println(
        psramFound()
            ? "YES"
            : "NO");

    Serial.print("[CAMERA] Free PSRAM: ");
    Serial.println(
        ESP.getFreePsram());

    Serial.print("[CAMERA] Free Heap: ");
    Serial.println(
        ESP.getFreeHeap());

    return true;
}

CameraFrame CameraService::capture()
{
    if (!initialized)
    {
        Serial.println("[CAMERA] Not initialized");
        return {};
    }

    release();

    Serial.println("[CAMERA] Capturing frame...");

    currentFrame = esp_camera_fb_get();

    if (currentFrame == nullptr)
    {
        Serial.println("[CAMERA] Capture failed");
        return {};
    }

    Serial.print("[CAMERA] Captured ");
    Serial.print(currentFrame->len);
    Serial.println(" bytes");

    return {
        .data = currentFrame->buf,
        .size = currentFrame->len
    };
}

void CameraService::release()
{
    if (currentFrame == nullptr)
    {
        return;
    }

    esp_camera_fb_return(currentFrame);
    currentFrame = nullptr;
}