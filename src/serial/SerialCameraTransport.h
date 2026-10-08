#pragma once

#include <Arduino.h>

class SerialCameraTransport
{
public:
    bool send(const uint8_t *data, size_t size);
};