#pragma once

#include "IMqttClient.h"
#include "../config/AppConfig.h"
#include <WiFi.h>
#include <PubSubClient.h>

class MqttClient : public IMqttClient
{
private:
    struct Subscription
    {
        const char *topic;
        MqttMessageHandler handler;
    };

    WiFiClient wifiClient;
    PubSubClient mqttClient;
    uint32_t lastReconnectAttempt = 0;

    Subscription subscriptions[AppConfig::Mqtt::MAX_SUBSCRIPTIONS];
    uint8_t subscriptionCount = 0;

    void connect();
    void onMessage(char *topic, byte *payload, unsigned int length);

public:
    MqttClient();

    void begin() override;
    void loop() override;
    bool subscribe(const char *topic, MqttMessageHandler handler) override;
    bool publish(const char *topic, const char *message) override;
};
