#include "AccessIndicator.h"
#include "config/PinConfig.h"
#include "config/AppConfig.h"

#include <Arduino.h>

void AccessIndicator::begin()
{
    pinMode(PinConfig::Indicator::GREEN_LED, OUTPUT);
    pinMode(PinConfig::Indicator::RED_LED, OUTPUT);
    pinMode(PinConfig::Indicator::BUZZER, OUTPUT);

    locked();
}

void AccessIndicator::locked()
{
    digitalWrite(PinConfig::Indicator::GREEN_LED, LOW);
    digitalWrite(PinConfig::Indicator::RED_LED, HIGH);
    digitalWrite(PinConfig::Indicator::BUZZER, LOW);
}

void AccessIndicator::unlocked()
{
    digitalWrite(PinConfig::Indicator::RED_LED, LOW);
    digitalWrite(PinConfig::Indicator::GREEN_LED, HIGH);

    digitalWrite(PinConfig::Indicator::BUZZER, HIGH);
    delay(AppConfig::Indicator::BEEP_DURATION);
    digitalWrite(PinConfig::Indicator::BUZZER, LOW);
}

void AccessIndicator::accessDenied()
{
    digitalWrite(PinConfig::Indicator::GREEN_LED, LOW);

    for (int i = 0; i < 3; ++i)
    {
        digitalWrite(PinConfig::Indicator::RED_LED, HIGH);
        delay(AppConfig::Indicator::LED_ON_DURATION);

        digitalWrite(PinConfig::Indicator::BUZZER, HIGH);
        delay(AppConfig::Indicator::BEEP_DURATION);

        digitalWrite(PinConfig::Indicator::BUZZER, LOW);
        delay(AppConfig::Indicator::BEEP_INTERVAL);

        digitalWrite(PinConfig::Indicator::RED_LED, LOW);
        delay(AppConfig::Indicator::LED_OFF_DURATION);
    }

    locked();
}

void AccessIndicator::off()
{
    digitalWrite(PinConfig::Indicator::GREEN_LED, LOW);
    digitalWrite(PinConfig::Indicator::RED_LED, LOW);
    digitalWrite(PinConfig::Indicator::BUZZER, LOW);
}