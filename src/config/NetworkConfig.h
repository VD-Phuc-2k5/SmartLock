#pragma once

#include <Arduino.h>
#include <cstdint>
#include "AppConfig.h"

struct NetworkConfig
{
    String ssid;
    String password;
    String mqttHost;
    uint16_t mqttPort = AppConfig::Mqtt::PORT;

    bool isValid() const
    {
        return ssid.length() > 0 &&
               mqttHost.length() > 0 &&
               mqttPort > 0;
    }
};