#include "DoorLock.h"
#include "config/PinConfig.h"

#include <Arduino.h>

namespace
{
    constexpr uint32_t SERVO_FREQUENCY = 50;
    constexpr uint8_t SERVO_RESOLUTION = 14;
    constexpr uint32_t SERVO_PERIOD_US = 20000;
    constexpr uint32_t SERVO_MIN_PULSE_US = 500;
    constexpr uint32_t SERVO_MAX_PULSE_US = 2400;
}

void DoorLock::begin()
{
    if (!ledcAttach(
            PinConfig::Lock::SERVO,
            SERVO_FREQUENCY,
            SERVO_RESOLUTION))
    {
        Serial.println("[DOOR] failed to attach PWM");
        return;
    }

    lock();
}

void DoorLock::lock()
{
    writePosition(LOCK_POSITION);
}

void DoorLock::unlock()
{
    writePosition(UNLOCK_POSITION);
}

void DoorLock::writePosition(int position)
{
    position = constrain(position, 0, 180);
    const uint32_t pulseWidth = map(
        position,
        0,
        180,
        SERVO_MIN_PULSE_US,
        SERVO_MAX_PULSE_US);
    const uint32_t duty = (pulseWidth * ((1UL << SERVO_RESOLUTION) - 1)) / SERVO_PERIOD_US;
    ledcWrite(PinConfig::Lock::SERVO, duty);
}