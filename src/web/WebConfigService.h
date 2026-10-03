#pragma once

#include "../config/ConfigManager.h"
#include <WebServer.h>

class WebConfigService
{
private:
    ConfigManager &configManager;
    WebServer server;

    void handleRoot();
    void handleSave();
    void handleReset();

public:
    explicit WebConfigService(ConfigManager &manager);

    bool begin();
    void handleClient();
};