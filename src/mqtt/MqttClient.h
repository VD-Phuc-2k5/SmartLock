#pragma once

#include "IMqttClient.h"
#include "../config/AppConfig.h"
#include "../config/ConfigManager.h"
#include "../serial/SerialMqttTransport.h"

class MqttClient : public IMqttClient
{
private:
    struct Subscription
    {
        const char *topic;
        MqttMessageHandler handler;
    };

    ConfigManager &configManager;
    SerialMqttTransport serialTransport;

    Subscription subscriptions[
        AppConfig::Mqtt::MAX_SUBSCRIPTIONS
    ];

    uint8_t subscriptionCount = 0;

    void onMessage(
        const char *topic,
        const char *payload);

public:
    explicit MqttClient(ConfigManager &manager);

    void begin() override;

    void loop() override;

    bool subscribe(
        const char *topic,
        MqttMessageHandler handler) override;

    bool publish(
        const char *topic,
        const char *message) override;
};
