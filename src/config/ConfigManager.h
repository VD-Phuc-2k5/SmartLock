#pragma once

#include <Preferences.h>
#include "NetworkConfig.h"

class ConfigManager
{
private:
    Preferences preferences;
    NetworkConfig config;

public:
    bool begin();
    bool load();
    bool save(const NetworkConfig &newConfig);
    void clear();

    bool hasConfig() const;
    const NetworkConfig &get() const;
};