#include "InitializeSyncServer.hpp"

InitializeSyncServer::InitializeSyncServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore)
    : port(port), ssid(ssid), password(password), server(port), wifiSemaphore(wifiSemaphore) {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
}

void InitializeSyncServer::start(void) {
    // Определяем обработчики запросов
    server.on("/", [this]() { handleRoot(); });
    server.on("/connect", HTTP_POST, [this]() { handleConnect(); });

    // Запускаем сервер
    server.begin();
    Serial.println("Web server started!");
}

void InitializeSyncServer::handleRoot() {
    String html = generate_html_page();
    server.send(200, "text/html", html);
}

void InitializeSyncServer::handleConnect() {
    if (server.hasArg("ssid")) {
        settings.set_ssid(server.arg("ssid").c_str());
        status++;
    }
    if (server.hasArg("password")) {
        settings.set_password(server.arg("password").c_str());
        status++;
    }
    if (status == 2) {
        settings.is_set = 1;
        xSemaphoreGive(wifiSemaphore); // Освобождаем семафор
        server.send(200, "text/html", "<h1>WiFi credentials received!</h1>");
        Serial.print("Received SSID: ");
        Serial.println(settings.ssid);
        Serial.print("Received Password: ");
        Serial.println(settings.password);
        server.stop()
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