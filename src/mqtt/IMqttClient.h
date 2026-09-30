#pragma once

class IMqttClient
{
public:
    virtual ~IMqttClient() = default;

    virtual void begin() = 0;
    virtual void loop() = 0;
};