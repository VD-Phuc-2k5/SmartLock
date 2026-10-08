#include "SerialMqttTransport.h"

#include <cstring>

void SerialMqttTransport::begin()
{
    bufferLength = 0;

    Serial.println(
        "[SERIAL-MQTT] Transport initialized");
}

void SerialMqttTransport::loop(
    SerialMqttMessageHandler handler)
{
    while (Serial.available() > 0)
    {
        const char c =
            static_cast<char>(
                Serial.read());

        if (c == '\n')
        {
            buffer[bufferLength] = '\0';

            if (bufferLength > 0)
            {
                processLine(
                    buffer,
                    handler);
            }

            bufferLength = 0;

            continue;
        }

        if (c == '\r')
        {
            continue;
        }

        if (
            bufferLength <
            BUFFER_SIZE - 1)
        {
            buffer[bufferLength++] = c;
        }
        else
        {
            bufferLength = 0;

            Serial.println(
                "[SERIAL-MQTT] Buffer overflow");
        }
    }
}

void SerialMqttTransport::processLine(
    const char *line,
    SerialMqttMessageHandler handler)
{
    // Gateway -> ESP32:
    //
    // @MSG<TAB>topic<TAB>payload

    constexpr char PREFIX[] = "@MSG\t";

    constexpr size_t PREFIX_LENGTH =
        sizeof(PREFIX) - 1;

    if (
        std::strncmp(
            line,
            PREFIX,
            PREFIX_LENGTH) != 0)
    {
        // Ignore normal Serial debug logs.
        return;
    }

    const char *topicStart =
        line + PREFIX_LENGTH;

    const char *separator =
        std::strchr(
            topicStart,
            '\t');

    if (separator == nullptr)
    {
        Serial.println(
            "[SERIAL-MQTT] Invalid message");

        return;
    }

    const size_t topicLength =
        separator - topicStart;

    if (topicLength == 0)
    {
        return;
    }

    char topic[256];

    if (
        topicLength >=
        sizeof(topic))
    {
        Serial.println(
            "[SERIAL-MQTT] Topic too long");

        return;
    }

    std::memcpy(
        topic,
        topicStart,
        topicLength);

    topic[topicLength] = '\0';

    const char *payload =
        separator + 1;

    if (handler != nullptr)
    {
        handler(
            topic,
            payload);
    }
}

bool SerialMqttTransport::publish(
    const char *topic,
    const char *payload)
{
    if (
        topic == nullptr ||
        payload == nullptr)
    {
        return false;
    }

    // ESP32 -> Gateway:
    //
    // @PUB<TAB>topic<TAB>payload

    Serial.print("@PUB\t");
    Serial.print(topic);
    Serial.print('\t');
    Serial.println(payload);

    return true;
}