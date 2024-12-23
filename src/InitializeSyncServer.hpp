#ifndef INITIALIZE_SYNC_SERVER_HH
#define INITIALIZE_SYNC_SERVER_HH

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Settings.hpp"

class InitializeSyncServer
{
public:
  int status = 0;
  const char *ssid;
  const char *password;
  Settings settings;
  WebServer server; // Используем обычный WebServer
  int wifi_status = 0;

  // Конструктор с семафором
  InitializeSyncServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore);
  void start(void);
  
  void handleConnect();

private:
  int port = 0;
  SemaphoreHandle_t wifiSemaphore;

  void handleRoot();
  String generate_html_page();
  String get_wifi_option();
};

#endif