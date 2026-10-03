#include "MqttClient.h"
#include <cstring>

MqttClient::MqttClient(
    ConfigManager &manager)
    : configManager(manager),
      mqttClient(wifiClient)
{
}

void MqttClient::begin()
{
    if (!configManager.hasConfig())
    {
        Serial.println(
            "[MQTT] No network configuration");

        return;
    }

    const NetworkConfig &config =
        configManager.get();

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            "[MQTT] WiFi is not connected");

        return;
    }

    Serial.println(
        "Connected to WiFi");

    Serial.print("ESP32 IP: ");
    Serial.println(
        WiFi.localIP());

    Serial.print("Gateway: ");
    Serial.println(
        WiFi.gatewayIP());

    Serial.print("MQTT Broker: ");
    Serial.print(config.mqttHost);

    Serial.print(":");
    Serial.println(config.mqttPort);

    mqttClient.setServer(
        config.mqttHost.c_str(),
        config.mqttPort);

    mqttClient.setCallback(
        [this](
            char *topic,
            byte *payload,
            unsigned int length)
        {
            onMessage(
                topic,
                payload,
                length);
        });

    connect();
}

void MqttClient::loop()
{
    if (!configManager.hasConfig())
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if (!mqttClient.connected())
    {
        uint32_t now = millis();

        if (
            now - lastReconnectAttempt >=
            AppConfig::Mqtt::RECONNECT_INTERVAL_MS)
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
    Serial.println(
        "Connecting to MQTT broker...");

    if (
        mqttClient.connect(
            AppConfig::Mqtt::CLIENT_ID))
    {
        Serial.println(
            "Connected to MQTT broker");

        for (
            uint8_t i = 0;
            i < subscriptionCount;
            i++)
        {
            mqttClient.subscribe(
                subscriptions[i].topic);
        }
    }
    else
    {
        Serial.print(
            "Failed to connect to MQTT broker, rc=");

        Serial.println(
            mqttClient.state());
    }
}

bool MqttClient::subscribe(
    const char *topic,
    MqttMessageHandler handler)
{
    if (
        subscriptionCount >=
        AppConfig::Mqtt::MAX_SUBSCRIPTIONS)
    {
        return false;
    }

    subscriptions[subscriptionCount] =
        {topic, handler};

    subscriptionCount++;

    if (mqttClient.connected())
    {
        mqttClient.subscribe(topic);
    }

    return true;
}

bool MqttClient::publish(
    const char *topic,
    const char *message)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    return mqttClient.publish(
        topic,
        message);
}

void MqttClient::onMessage(
    char *topic,
    byte *payload,
    unsigned int length)
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
                reinterpret_cast<const char *>(
                    payload),
                length);

            return;
        }
    }
}
