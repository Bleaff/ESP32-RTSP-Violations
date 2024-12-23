#ifndef INITIALIZE_SETTINGS_SERVER_HPP
#define INITIALIZE_SETTINGS_SERVER_HPP

#include <Arduino.h>
#include <WiFi.h>
#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_DELETE
#include <ESPAsyncWebServer.h>
#include "Settings.hpp"

class InitializeSettingsServer {
public:
    InitializeSettingsServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore);
    void start(void);

private:
    int port;
    const char *ssid;
    const char *password;
    TaskHandle_t Task1;
    AsyncWebServer server;
    SemaphoreHandle_t wifiSemaphore;
    Settings settings;

    void handleConnect(AsyncWebServerRequest *request);
    void start_monitor_thread(void);
    static void monitor_network_connection(void *params);
    String generate_html_page();
    String get_wifi_option();
};

#endif
