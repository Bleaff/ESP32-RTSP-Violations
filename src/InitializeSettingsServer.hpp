#ifndef INITIALIZE_SETTINGS_SERVER_HPP
#define INITIALIZE_SETTINGS_SERVER_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Settings.hpp"

class InitializeSettingsServer
{
public:
  int status = 0;
  int wifi_status = 0;
  const char *ssid;
  const char *password;
  Settings settings;


  // Конструктор с семафором
  InitializeSettingsServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore);
  InitializeSettingsServer(const InitializeSettingsServer &settings);
  void start(void);

private:
  int port = 0;
  TaskHandle_t Task1;
  AsyncWebServer server;

  // Семафор для синхронизации WiFi
  SemaphoreHandle_t wifiSemaphore;

  void start_monitor_thread(void);
  static void monitor_network_connection(void *params);
  String generate_html_page();
  String get_wifi_option();
};

#endif