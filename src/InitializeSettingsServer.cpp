#include "InitializeSettingsServer.hpp"

InitializeSettingsServer::InitializeSettingsServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore)
    : port(port), ssid(ssid), password(password), server(port), wifiSemaphore(wifiSemaphore) {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
}

void InitializeSettingsServer::start(void) {
    // Define the root route
    this->server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String html = generate_html_page();
        request->send(200, "text/html", html);
    });

    // Define the connect route
    this->server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleConnect(request);
    });

    // Start the monitor thread for connection attempts
    start_monitor_thread();

    // Start the web server
    this->server.begin();
    Serial.println("WiFi setup server started.");
}

void InitializeSettingsServer::handleConnect(AsyncWebServerRequest *request) {
    bool validRequest = true;

    if (request->hasParam("ssid", true)) {
        settings.set_ssid(request->getParam("ssid", true)->value().c_str());
    } else {
        validRequest = false;
    }

    if (request->hasParam("password", true)) {
        settings.set_password(request->getParam("password", true)->value().c_str());
    } else {
        validRequest = false;
    }

    if (validRequest) {
        settings.is_set = 1;
        xSemaphoreGive(this->wifiSemaphore);
        Serial.printf("Connecting to SSID: %s\nPassword: %s\n", settings.ssid, settings.password);
        request->send(200, "text/html", "<h1>Trying to connect to WiFi...</h1>");
    } else {
        String html = generate_html_page();
        request->send(400, "text/html", html);
    }
}

String InitializeSettingsServer::get_wifi_option() {
    String wifiOptions = "<option value=\"none\">No networks found</option>";
    int n = WiFi.scanNetworks();
    if (n > 0) {
        wifiOptions = "";
        for (int i = 0; i < n; ++i) {
            wifiOptions += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
        }
    }
    return wifiOptions;
}

String InitializeSettingsServer::generate_html_page() {
    String wifiOptions = get_wifi_option();
    return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 WiFi Setup</title>
        <style>
            body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background-color: #f0f4f8; }
            .container { max-width: 400px; margin: 50px auto; padding: 20px; background: #fff; border-radius: 8px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1); }
            h1 { color: #333; }
            label, select, input { display: block; width: 100%; margin-bottom: 15px; }
            input[type=\"submit\"] { background-color: #4CAF50; color: white; border: none; padding: 10px; cursor: pointer; }
            input[type=\"submit\"]:hover { background-color: #45a049; }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>WiFi Configuration</h1>
            <form action="/connect" method="post">
                <label for="ssid">Select WiFi Network:</label>
                <select name="ssid" id="ssid">
                    )rawliteral" + wifiOptions + R"rawliteral(
                </select>
                <label for="password">Password:</label>
                <input type="password" id="password" name="password">
                <input type="submit" value="Connect">
            </form>
        </div>
    </body>
    </html>
    )rawliteral";
}

void InitializeSettingsServer::monitor_network_connection(void *params) {
    InitializeSettingsServer *instance = static_cast<InitializeSettingsServer *>(params);
    Serial.println("Monitoring WiFi connection attempts...");

    while (true) {
        if (instance->settings.is_set) {
            instance->server.end(); // Stop the server after successful configuration
            WiFi.disconnect();
            WiFi.begin(instance->settings.ssid, instance->settings.password);

            unsigned long startAttemptTime = millis();
            const unsigned long timeout = 10000; // 10 seconds

            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
                delay(500);
                Serial.print(".");
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nWiFi connected successfully!");
                Serial.println("IP Address: " + WiFi.localIP().toString());
                xSemaphoreGive(instance->wifiSemaphore);
            } else {
                Serial.println("\nFailed to connect to WiFi.");
            }
            break;
        }
        delay(100);
    }
    vTaskDelete(NULL);
}

void InitializeSettingsServer::start_monitor_thread() {
    xTaskCreatePinnedToCore(
        monitor_network_connection,   // Task function
        "MonitorWiFi",               // Name of task
        8182,                         // Stack size of task
        this,                         // Pass the instance as the task parameter
        1,                            // Priority of the task
        &Task1,                       // Task handle to keep track of created task
        0                             // Core where the task should run
    );
}
