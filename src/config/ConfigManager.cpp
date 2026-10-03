#include "ConfigManager.h"

bool ConfigManager::begin()
{
    return load();
}

bool ConfigManager::load()
{
    if (!preferences.begin("network", true))
    {
        Serial.println("[CONFIG] Failed to open Preferences");
        config = {};
        return false;
    }

    config.ssid =
        preferences.getString("ssid", "");

    config.password =
        preferences.getString("password", "");

    config.mqttHost =
        preferences.getString("mqttHost", "");

    config.mqttPort =
        preferences.getUShort(
            "mqttPort",
            AppConfig::Mqtt::PORT);

    preferences.end();

    return config.isValid();
}

bool ConfigManager::save(
    const NetworkConfig &newConfig)
{
    if (!newConfig.isValid())
    {
        return false;
    }

    if (!preferences.begin("network", false))
    {
        Serial.println(
            "[CONFIG] Failed to open Preferences for write");

        return false;
    }

    preferences.putString(
        "ssid",
        newConfig.ssid);

    preferences.putString(
        "password",
        newConfig.password);

    preferences.putString(
        "mqttHost",
        newConfig.mqttHost);

    preferences.putUShort(
        "mqttPort",
        newConfig.mqttPort);

    preferences.end();

    config = newConfig;

    return true;
}

void ConfigManager::clear()
{
    if (!preferences.begin("network", false))
    {
        Serial.println(
            "[CONFIG] Failed to open Preferences");

        return;
    }

    preferences.clear();
    preferences.end();

    config = {};

    Serial.println(
        "[CONFIG] Configuration cleared");
}

bool ConfigManager::hasConfig() const
{
    return config.isValid();
}

const NetworkConfig &
ConfigManager::get() const
{
    return config;
}