#pragma once

#include <functional>

using MqttMessageHandler = std::function<void(const char *payload, unsigned int length)>;

class IMqttClient
{
public:
    virtual ~IMqttClient() = default;

    virtual void begin() = 0;
    virtual void loop() = 0;
    virtual bool subscribe(const char *topic, MqttMessageHandler handler) = 0;
    virtual bool publish(const char *topic, const char *message) = 0;
};
