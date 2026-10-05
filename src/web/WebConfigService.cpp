#include "WebConfigService.h"

#include <WiFi.h>

WebConfigService::WebConfigService(ConfigManager &manager)
    : configManager(manager),
      server(80)
{
    Serial.println("[WEB] Constructor");
}

void WebConfigService::handleRoot()
{
    const NetworkConfig &config = configManager.get();

    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Smart Lock Setup</title>

    <style>
        :root {
            --primary: #0284c7;
            --primary-hover: #0369a1;
        }

        body {
            font-family: Arial, sans-serif;
            max-width: 480px;
            margin: 40px auto;
            padding: 20px;
        }

        h1 {
            font-size: 24px;
            color: var(--primary);
        }

        label {
            display: block;
            margin-top: 16px;
            font-weight: 600;
        }

        input {
            box-sizing: border-box;
            width: 100%;
            padding: 12px;
            margin-top: 6px;
            border: 1px solid #ccc;
            border-radius: 8px;
            font-size: 16px;
            outline: none;
        }

        input:focus {
            border-color: var(--primary);
            box-shadow: 0 0 0 2px rgba(2, 132, 199, 0.15);
        }

        .password-wrapper {
            position: relative;
        }

        .password-wrapper input {
            padding-right: 70px;
        }

        .toggle-password {
            position: absolute;
            right: 8px;
            top: 50%;
            transform: translateY(-50%);
            width: auto;
            margin: 0;
            padding: 6px 8px;
            border: none;
            background: transparent;
            color: var(--primary);
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
        }

        .toggle-password:hover {
            color: var(--primary-hover);
        }

        button.submit {
            width: 100%;
            padding: 12px;
            margin-top: 22px;
            border: 0;
            border-radius: 8px;
            background: var(--primary);
            color: white;
            font-size: 16px;
            cursor: pointer;
        }

        button.submit:hover {
            background: var(--primary-hover);
        }

        .reset {
            width: 100%;
            padding: 12px;
            margin-top: 12px;
            border: 0;
            border-radius: 8px;
            background: #c62828;
            color: white;
            font-size: 16px;
            cursor: pointer;
        }
    </style>
</head>

<body>

    <h1>Smart Lock Setup</h1>

    <form method="POST" action="/save">

        <label>Wi-Fi SSID</label>

        <input
            name="ssid"
            required
            value=")rawliteral";

    html += config.ssid;

    html += R"rawliteral(">

        <label>Wi-Fi Password</label>

        <div class="password-wrapper">

            <input
                id="password"
                type="password"
                name="password"
                value=")rawliteral";

    html += config.password;

    html += R"rawliteral(">

            <button
                type="button"
                class="toggle-password"
                onclick="togglePassword()">
                Hiện
            </button>

        </div>

        <label>MQTT Broker</label>

        <input
            name="mqttHost"
            required
            value=")rawliteral";

    html += config.mqttHost;

    html += R"rawliteral(">

        <label>MQTT Port</label>

        <input
            type="number"
            name="mqttPort"
            min="1"
            max="65535"
            value=")rawliteral";

    html += String(config.mqttPort);

    html += R"rawliteral(">

        <button
            type="submit"
            class="submit">
            Save & Restart
        </button>

    </form>

    <form method="POST" action="/reset">

        <button
            class="reset"
            type="submit">
            Clear Configuration
        </button>

    </form>

    <script>
        function togglePassword() {
            const password =
                document.getElementById("password");

            const button =
                document.querySelector(".toggle-password");

            if (password.type === "password") {
                password.type = "text";
                button.textContent = "Ẩn";
            } else {
                password.type = "password";
                button.textContent = "Hiện";
            }
        }
    </script>

</body>
</html>
)rawliteral";

    server.send(200, "text/html", html);
}

void WebConfigService::handleSave()
{
    Serial.println("[WEB] ===== SAVE =====");

    NetworkConfig newConfig;

    newConfig.ssid = server.arg("ssid");
    newConfig.password = server.arg("password");
    newConfig.mqttHost = server.arg("mqttHost");

    long port = server.arg("mqttPort").toInt();

    Serial.print("[WEB] SSID: ");
    Serial.println(newConfig.ssid);

    Serial.print("[WEB] MQTT Host: ");
    Serial.println(newConfig.mqttHost);

    Serial.print("[WEB] MQTT Port: ");
    Serial.println(port);

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

    Serial.println("[WEB] Configuration saved");
    Serial.println("[WEB] Restarting...");

    delay(1000);

    ESP.restart();
}

void WebConfigService::handleReset()
{
    Serial.println("[WEB] ===== RESET =====");

    configManager.clear();

    server.send(
        200,
        "text/html",
        "<h2>Configuration cleared.</h2>"
        "<p>ESP32 is restarting...</p>");

    Serial.println("[WEB] Configuration cleared");
    Serial.println("[WEB] Restarting...");

    delay(1000);

    ESP.restart();
}

bool WebConfigService::begin()
{
    Serial.println("[WEB] ===== BEGIN START =====");

    if (configManager.hasConfig())
    {
        Serial.println(
            "[WEB] Network configuration already exists");

        Serial.println(
            "[WEB] Setup AP skipped");

        Serial.println(
            "[WEB] ===== BEGIN END =====");

        return true;
    }

    Serial.println("[WEB] No network configuration");
    Serial.println("[WEB] Starting Setup AP...");

    const char *apName = "SmartLock-Setup";
    const char *apPassword = "12345678";

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);

    Serial.println("[WEB] WiFi mode: AP");

    WiFi.mode(WIFI_AP);

    delay(1000);

    Serial.println("[WEB] Calling softAP...");

    bool started =
        WiFi.softAP(
            apName,
            apPassword,
            1,
            false,
            4);

    if (!started)
    {
        Serial.println("[WEB] AP START FAILED");

        WiFi.mode(WIFI_OFF);

        return false;
    }

    delay(300);

    Serial.println("[WEB] AP started");

    Serial.print("[WEB] SSID: ");
    Serial.println(apName);

    Serial.print("[WEB] IP: ");
    Serial.println(WiFi.softAPIP());

    server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            Serial.println("[WEB] GET /");
            handleRoot();
        });

    server.on(
        "/save",
        HTTP_POST,
        [this]()
        {
            Serial.println("[WEB] POST /save");
            handleSave();
        });

    server.on(
        "/reset",
        HTTP_POST,
        [this]()
        {
            Serial.println("[WEB] POST /reset");
            handleReset();
        });

    server.onNotFound(
        [this]()
        {
            Serial.println("[WEB] 404");

            server.send(
                404,
                "text/plain",
                "Not found");
        });

    server.begin();

    Serial.println("[WEB] Server started");
    Serial.println("[WEB] ===== BEGIN END =====");

    return true;
}

void WebConfigService::handleClient()
{
    server.handleClient();
}