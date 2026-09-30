#include "MqttClient.h"

namespace
{
    constexpr char WIFI_SSID[] = "Wokwi-GUEST";
    constexpr char WIFI_PASSWORD[] = "";

    constexpr char MQTT_BROKER[] = "host.wokwi.internal";
    constexpr uint16_t MQTT_PORT = 1883;

    constexpr char MQTT_TOPIC[] =
        "smartlock/device/esp32-01/otp";

    constexpr char MQTT_CLIENT_ID[] = "smartlock-esp32-01";

    constexpr uint32_t WIFI_TIMEOUT_MS = 15000;
    constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
}

MqttClient::MqttClient()
    : mqttClient(wifiClient)
{
}

void MqttClient::begin()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start >= WIFI_TIMEOUT_MS)
        {
            Serial.println("Failed to connect to WiFi");
            return;
        }
        delay(500);
    }

    Serial.println("Connected to WiFi");

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
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
        if (now - lastReconnectAttempt >= RECONNECT_INTERVAL_MS)
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
    if (mqttClient.connect(MQTT_CLIENT_ID))
    {
        Serial.println("Connected to MQTT broker");
        mqttClient.subscribe(MQTT_TOPIC);
    }
    else
    {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.print(mqttClient.state());
    }
}

void MqttClient::onMessage(char *topic, byte *payload, unsigned int length)
{
    (void)topic;
    unsigned int n = length < sizeof(lastMessage) - 1 ? length : sizeof(lastMessage) - 1;
    memcpy(lastMessage, payload, n);
    lastMessage[n] = '\0';
    hasNewMessage = true;
}

bool MqttClient::hasMessage()
{
    return hasNewMessage;
}

const char *MqttClient::getMessage()
{
    return lastMessage;
}

void MqttClient::clearMessage()
{
    hasNewMessage = false;
}
