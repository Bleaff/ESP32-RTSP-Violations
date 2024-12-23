#include "InitializeSettingsServer.hpp"
#include <WiFi.h>
#include <freertos/semphr.h>

/** Semaphore for WiFi synchronization */
SemaphoreHandle_t wifiSemaphore;

/** InitializeSyncServer instance */
InitializeSyncServer *WifiServer;

void setup() {
    Serial.begin(115200);

    // Создаем семафор
    wifiSemaphore = xSemaphoreCreateBinary();

    // Создаем объект InitializeSyncServer и передаем семафор
    WifiServer = new InitializeSyncServer(80, "ESP32_SETUP", "12345678", wifiSemaphore);

    // Запускаем сервер
    WifiServer->start();

    // Ожидаем подключения к Wi-Fi
    Serial.println("Waiting for WiFi credentials...");
    while (!WifiServer->wifi_status)
      delay(20);
    // xSemaphoreTake(wifiSemaphore, portMAX_DELAY); // Ожидаем освобождения семафора
    Serial.println("WiFi credentials received! Connecting to WiFi...");
}

void loop() {
    // Обрабатываем запросы на сервере
    WifiServer->server.handleClient();
    delay(100);
}