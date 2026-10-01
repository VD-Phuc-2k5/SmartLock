#include "DoorLock.h"
#include "config/PinConfig.h"

#include <Arduino.h>
#include <ESP32Servo.h>

namespace
{
    Servo servo;
}

void DoorLock::begin()
{
    servo.setPeriodHertz(50);

    servo.attach(
        PinConfig::Lock::SERVO,
        500,
        2400);
}

void DoorLock::lock()
{
    servo.write(LOCK_POSITION);
}

void DoorLock::unlock()
{
    servo.write(UNLOCK_POSITION);
}