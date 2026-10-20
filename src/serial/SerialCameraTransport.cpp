#include "SerialCameraTransport.h"

bool SerialCameraTransport::send(
    const uint8_t *data,
    size_t size,
    bool finalCapture)
{
    if (data == nullptr || size == 0)
    {
        return false;
    }

    Serial.print("@CAM\t");
    Serial.print(size);

    if (finalCapture)
    {
        Serial.print("\tFINAL");
    }

    Serial.println();

    const size_t written =
        Serial.write(data, size);

    Serial.flush();

    return written == size;
}