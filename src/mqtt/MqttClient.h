#pragma once

#include "IMqttClient.h"
#include <WiFi.h>
#include <PubSubClient.h>

class MqttClient : public IMqttClient
{
private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    uint32_t lastReconnectAttempt = 0;

    char lastMessage[64] = {0};
    bool hasNewMessage = false;

    void connect();
    void onMessage(char* topic, byte* payload, unsigned int length);

public:
    MqttClient();

    void begin() override;
    void loop() override;

    bool hasMessage();
    const char* getMessage();
    void clearMessage();
};
