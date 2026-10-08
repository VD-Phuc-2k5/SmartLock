#pragma once

#include <Arduino.h>
#include <functional>

using SerialMqttMessageHandler =
    std::function<void(
        const char *topic,
        const char *payload)>;

class SerialMqttTransport
{
private:
    static constexpr size_t BUFFER_SIZE = 512;

    char buffer[BUFFER_SIZE];
    size_t bufferLength = 0;

    void processLine(
        const char *line,
        SerialMqttMessageHandler handler);

public:
    void begin();

    void loop(
        SerialMqttMessageHandler handler);

    bool publish(
        const char *topic,
        const char *payload);
};