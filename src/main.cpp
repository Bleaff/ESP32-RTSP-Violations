#include <Arduino.h>
#include "InitializeSyncServer.hpp"
#include "main.h"

// Semaphore for WiFi synchronization
SemaphoreHandle_t wifiSemaphore;

OV2640 cam;

TaskHandle_t wifiServerTask;

// Function to handle WiFi server processing
void wifiServerTaskFunction(void *parameter) {
    InitializeSyncServer *wifiServer = static_cast<InitializeSyncServer *>(parameter);
    for (;;) {
        wifiServer->server.handleClient(); // Process incoming client requests
        vTaskDelay(pdMS_TO_TICKS(10)); // Prevent tight looping
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting ESP32 WiFi Setup");

    // Create the semaphore
    wifiSemaphore = xSemaphoreCreateBinary();

    // Initialize the WiFi setup server
    InitializeSyncServer *wifiServer = new InitializeSyncServer(80, "ESP32_SETUP", "12345678", wifiSemaphore);
    wifiServer->start();

    // Create a task for handling the WiFi server on the second core
    xTaskCreatePinnedToCore(
        wifiServerTaskFunction,      // Task function
        "WiFiServerTask",          // Name of the task
        4096,                       // Stack size
        wifiServer,                 // Task parameter (pass the server instance)
        1,                          // Task priority
        &wifiServerTask,            // Task handle
        1                           // Core 1
    );

    // Wait for WiFi configuration to complete
    Serial.println("Waiting for WiFi connection...");
    xSemaphoreTake(wifiSemaphore, portMAX_DELAY);

    // WiFi is now connected
    Serial.println("WiFi connected successfully!");
    Serial.print("Device IP: ");
    Serial.println(WiFi.localIP());

    // Stop the WiFi server task and free resources
    vTaskDelete(wifiServerTask); // Delete the WiFi server task
    delete wifiServer;          // Free memory allocated for the server

    // Start your main application logic here
    Serial.println("Starting main application...");

    // Start the serial connection  
    Serial.println("\n\n##################################");
    Serial.printf("Internal Total heap %d, internal Free Heap %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
    Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
    Serial.printf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    Serial.printf("Flash Size %d, Flash Speed %d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    Serial.println("##################################\n\n");

    // Initialize the ESP32 CAM, here we use the AIthinker ESP32 CAM
    delay(100);
    cam.init(esp32cam_aithinker_config);
    delay(100);
    IPAddress ip = WiFi.localIP();
    Serial.print("Stream Link: rtsp://");
    Serial.print(ip);
    Serial.println(":8554/mjpeg/1\n");
    initRTSP();
}

void loop() {
    delay(10);
}
