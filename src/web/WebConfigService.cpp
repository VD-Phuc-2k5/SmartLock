#include "WebConfigService.h"

#include <WiFi.h>
#include <LittleFS.h>

WebConfigService::WebConfigService(ConfigManager &manager)
    : configManager(manager),
      server(80)
{
}

void WebConfigService::handleSave()
{
    NetworkConfig newConfig;

    newConfig.ssid = server.arg("ssid");
    newConfig.password = server.arg("password");
    newConfig.mqttHost = server.arg("mqttHost");

    long port = server.arg("mqttPort").toInt();

    if (port < 1 || port > 65535)
    {
        server.send(
            400,
            "text/html",
            "<h2>Invalid MQTT port.</h2>"
            "<a href=\"/\">Back</a>");

        return;
    }

    newConfig.mqttPort =
        static_cast<uint16_t>(port);

    if (!configManager.save(newConfig))
    {
        server.send(
            400,
            "text/html",
            "<h2>Invalid configuration.</h2>"
            "<a href=\"/\">Back</a>");

        return;
    }

    server.send(
        200,
        "text/html",
        "<h2>Configuration saved.</h2>"
        "<p>ESP32 is restarting...</p>");

    delay(1000);

    ESP.restart();
}

void WebConfigService::handleReset()
{
    configManager.clear();

    server.send(
        200,
        "text/html",
        "<h2>Configuration cleared.</h2>"
        "<p>ESP32 is restarting...</p>");

    delay(1000);

    ESP.restart();
}

bool WebConfigService::begin()
{
    WiFi.mode(WIFI_AP);

    const char *apName = "SmartLock-Setup";

    if (!WiFi.softAP(apName))
    {
        Serial.println(
            "[WEB] Failed to start configuration AP");

        return false;
    }

    Serial.println();
    Serial.println(
        "=== SMART LOCK CONFIGURATION ===");

    Serial.print("[WEB] SSID: ");
    Serial.println(apName);

    Serial.print("[WEB] IP: ");
    Serial.println(WiFi.softAPIP());

    server.serveStatic(
        "/",
        LittleFS,
        "/index.html");

    server.on(
        "/save",
        HTTP_POST,
        [this]()
        {
            handleSave();
        });

    server.on(
        "/reset",
        HTTP_POST,
        [this]()
        {
            handleReset();
        });

    server.onNotFound(
        [this]()
        {
            server.send(
                404,
                "text/plain",
                "Not found");
        });

    server.begin();

    Serial.println("[WEB] Server started");

    return true;
}

void WebConfigService::handleClient()
{
    server.handleClient();
}