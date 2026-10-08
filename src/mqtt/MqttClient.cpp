#include "MqttClient.h"
#include <cstring>

MqttClient::MqttClient(
    ConfigManager &manager)
    : configManager(manager)
{
}

void MqttClient::begin()
{
    Serial.println("[MQTT] Starting Serial MQTT transport...");

    serialTransport.begin();

    Serial.println("[MQTT] Serial MQTT transport ready");
}

void MqttClient::loop()
{
    serialTransport.loop(
        [this](
            const char *topic,
            const char *payload)
        {
            onMessage(topic, payload);
        });
}

bool MqttClient::subscribe(
    const char *topic,
    MqttMessageHandler handler)
{
    if (
        subscriptionCount >=
        AppConfig::Mqtt::MAX_SUBSCRIPTIONS)
    {
        Serial.println(
            "[MQTT] Subscription limit reached");

        return false;
    }

    subscriptions[subscriptionCount] = {
        topic,
        handler
    };

    subscriptionCount++;

    Serial.print("[MQTT] Subscribed: ");
    Serial.println(topic);

    return true;
}

bool MqttClient::publish(
    const char *topic,
    const char *message)
{
    return serialTransport.publish(
        topic,
        message);
}

void MqttClient::onMessage(
    const char *topic,
    const char *payload)
{
    for (
        uint8_t i = 0;
        i < subscriptionCount;
        i++)
    {
        if (
            std::strcmp(
                topic,
                subscriptions[i].topic) == 0)
        {
            subscriptions[i].handler(
                payload,
                std::strlen(payload));

            return;
        }
    }

    Serial.print("[MQTT] No handler for topic: ");
    Serial.println(topic);
}
