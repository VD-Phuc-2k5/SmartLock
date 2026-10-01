#include "MqttClient.h"

#include <cstring>
#include "config/AppConfig.h"

MqttClient::MqttClient()
    : mqttClient(wifiClient)
{
}

void MqttClient::begin()
{
    WiFi.begin(AppConfig::Wifi::SSID, AppConfig::Wifi::PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start >= AppConfig::Wifi::TIMEOUT_MS)
        {
            Serial.println("Failed to connect to WiFi");
            return;
        }
        delay(AppConfig::Mqtt::DELAY_MS);
    }

    Serial.println("Connected to WiFi");

    mqttClient.setServer(AppConfig::Mqtt::BROKER, AppConfig::Mqtt::PORT);
    mqttClient.setCallback(
        [this](char *topic, byte *payload, unsigned int length)
        {
            onMessage(topic, payload, length);
        });

    connect();
}

void MqttClient::loop()
{
    if (!mqttClient.connected())
    {
        uint32_t now = millis();
        if (now - lastReconnectAttempt >= AppConfig::Mqtt::RECONNECT_INTERVAL_MS)
        {
            lastReconnectAttempt = now;
            connect();
        }
        return;
    }

    mqttClient.loop();
}

void MqttClient::connect()
{
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect(AppConfig::Mqtt::CLIENT_ID))
    {
        Serial.println("Connected to MQTT broker");

        for (uint8_t i = 0; i < subscriptionCount; i++)
        {
            mqttClient.subscribe(subscriptions[i].topic);
        }
    }
    else
    {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.print(mqttClient.state());
    }
}

bool MqttClient::subscribe(const char *topic, MqttMessageHandler handler)
{
    if (subscriptionCount >= AppConfig::Mqtt::MAX_SUBSCRIPTIONS)
    {
        return false;
    }

    subscriptions[subscriptionCount] = {topic, handler};
    subscriptionCount++;

    if (mqttClient.connected())
    {
        mqttClient.subscribe(topic);
    }

    return true;
}

bool MqttClient::publish(const char *topic, const char *message)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    return mqttClient.publish(topic, message);
}

void MqttClient::onMessage(char *topic, byte *payload, unsigned int length)
{
    for (uint8_t i = 0; i < subscriptionCount; i++)
    {
        if (std::strcmp(topic, subscriptions[i].topic) == 0)
        {
            subscriptions[i].handler(
                reinterpret_cast<const char *>(payload),
                length);
            return;
        }
    }
}
