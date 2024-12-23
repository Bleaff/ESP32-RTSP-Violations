#include "InitializeSyncServer.hpp"

InitializeSyncServer::InitializeSyncServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore)
    : port(port), ssid(ssid), password(password), server(port), wifiSemaphore(wifiSemaphore) {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
}

void InitializeSyncServer::start(void) {
    // Define request handlers
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/connect", HTTP_POST, [this]() { handleConnect(); });

    // Start the server
    server.begin();
    Serial.println("Web server started!");
}

void InitializeSyncServer::handleRoot() {
    String html = generate_html_page();
    server.send(200, "text/html", html);
}

void InitializeSyncServer::handleConnect() {
    Serial.println("Received POST request to /connect");

    // Отображаем все аргументы для отладки
    for (int i = 0; i < server.args(); i++) {
        Serial.printf("Arg %s: %s\n", server.argName(i).c_str(), server.arg(i).c_str());
    }

    // Проверяем наличие аргументов
    if (server.hasArg("ssid")) {
        String ssid = server.arg("ssid");
        settings.set_ssid(ssid.c_str()); // Убедитесь, что метод set_ssid выделяет память корректно
        status++;
    }
    if (server.hasArg("password")) {
        String password = server.arg("password");
        settings.set_password(password.c_str()); // Убедитесь, что метод set_password выделяет память корректно
        status++;
    }

    // Проверяем, установлены ли оба параметра
    if (status == 2) {
        settings.is_set = 1;
        server.send(200, "text/html", "<h1>WiFi credentials received!</h1>");
        Serial.printf("Received SSID: %s\n", settings.ssid);
        Serial.printf("Received Password: %s\n", settings.password);

        server.stop();
        WiFi.disconnect();
        WiFi.begin(settings.ssid, settings.password);

        unsigned long startAttemptTime = millis();
        const unsigned long timeout = 10000; // 10 seconds

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
            delay(500);
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected successfully!");
            Serial.println("IP Address: " + WiFi.localIP().toString());
            xSemaphoreGive(wifiSemaphore);
        } else {
            Serial.println("\nFailed to connect to WiFi.");
        }
    } else {
        server.send(400, "text/html", "<h1>Missing SSID or Password!</h1>");
    }
}



String InitializeSyncServer::generate_html_page() {
    String wifiOptions = get_wifi_option();
    String htmlPage = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>ESP32 WiFi Setup</title>
        </head>
        <body>
            <h1>WiFi Configuration</h1>
            <form action="/connect" method="post">
                <label for="ssid">Select WiFi Network:</label>
                <select name="ssid" id="ssid">)rawliteral";
    htmlPage += wifiOptions;
    htmlPage += R"rawliteral(
                </select>
                <label for="password">Password:</label>
                <input type="password" id="password" name="password">
                <input type="submit" value="Connect">
            </form>
        </body>
        </html>
    )rawliteral";
    return htmlPage;
}

String InitializeSyncServer::get_wifi_option() {
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
