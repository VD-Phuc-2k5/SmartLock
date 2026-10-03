#include "ConfigManager.h"

bool ConfigManager::begin()
{
    return load();
}

bool ConfigManager::load()
{
    preferences.begin("network", true);

    config.ssid = preferences.getString("ssid", "");
    config.password = preferences.getString("password", "");
    config.mqttHost = preferences.getString("mqttHost", "");
    config.mqttPort = preferences.getUShort(
        "mqttPort",
        AppConfig::Mqtt::PORT);

    preferences.end();

    return config.isValid();
}

bool ConfigManager::save(const NetworkConfig &newConfig)
{
    if (!newConfig.isValid())
    {
        return false;
    }

    preferences.begin("network", false);

    preferences.putString("ssid", newConfig.ssid);
    preferences.putString("password", newConfig.password);
    preferences.putString("mqttHost", newConfig.mqttHost);
    preferences.putUShort("mqttPort", newConfig.mqttPort);

    preferences.end();

    config = newConfig;

    return true;
}

void ConfigManager::clear()
{
    preferences.begin("network", false);
    preferences.clear();
    preferences.end();

    config = {};
}

bool ConfigManager::hasConfig() const
{
    return config.isValid();
}

const NetworkConfig &ConfigManager::get() const
{
    return config;
}