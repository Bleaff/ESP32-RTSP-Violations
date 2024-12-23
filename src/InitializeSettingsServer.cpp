#include "InitializeSettingsServer.hpp"

InitializeSettingsServer::InitializeSettingsServer(int port, const char *ssid, const char *password, SemaphoreHandle_t wifiSemaphore)
    : port(port), ssid(ssid), password(password), server(port), wifiSemaphore(wifiSemaphore) {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP address: ");
    Serial.println(WiFi.softAPIP());
}

InitializeSettingsServer::InitializeSettingsServer(const InitializeSettingsServer &other)
    : port(other.port), ssid(other.ssid), password(other.password), server(other.port) {}

void InitializeSettingsServer::start(void) {
    this->server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
		String html = generate_html_page();
        request->send(200, "text/html", html);
    });

	this->server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("ssid", true)) {
		  settings.set_ssid(request->getParam("ssid", true)->value().c_str());
		  status++;
		}
		if (request->hasParam("password", true)) {
		  settings.set_password(request->getParam("password", true)->value().c_str());
		  status++;
		}
		if (status == 2){
      settings.is_set = 1;
      xSemaphoreGive(this->wifiSemaphore);
    }
    else{
      String html = generate_html_page();
      request->send(200, "text/html", html);
      return ;
    }
		  Serial.print("Connecting to SSID: ");
  		Serial.println(settings.ssid);
		  Serial.print("Password: ");
  		Serial.println(settings.password);
      status = 0;
		request->send(200, "text/html", "<h1>Trying to connect to WiFi...</h1>");
	});
	
	start_monitor_thread();
    this->server.begin();
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
    String htmlPage = R"rawliteral(
	<!DOCTYPE html>
	<html>
	<head>
	    <title>ESP32 WiFi Setup</title>
	    <style>
	        body { font-family: Arial, sans-serif; display: flex; align-items: center; justify-content: center; min-height: 100vh; margin: 0; background-color: #f0f4f8; }
	        .container { width: 90%; max-width: 400px; background-color: #ffffff; padding: 30px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); text-align: center; }
	        h1 { color: #333; font-size: 24px; margin-bottom: 20px; }
	        label { display: block; text-align: left; margin-bottom: 5px; font-weight: bold; color: #555; }
	        select, input[type="password"] { width: 100%; padding: 10px; margin: 10px 0 20px; border-radius: 5px; border: 1px solid #ccc; font-size: 16px; color: #333; }
	        input[type="submit"] { width: 100%; padding: 12px; font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; transition: background-color 0.3s; }
	        input[type="submit"]:hover { background-color: #45a049; }
	    </style>
	</head>
	<body>
	    <div class="container">
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
	    </div>
	</body>
	</html>
	)rawliteral";
    return htmlPage;
}

void InitializeSettingsServer::monitor_network_connection(void *params) {
	InitializeSettingsServer *instance = static_cast<InitializeSettingsServer *>(params);
	Serial.println("Waiting for the password...");
	while (true){
		if (instance->settings.is_set) {
			instance->server.end();
			WiFi.disconnect();
			WiFi.begin(instance->settings.ssid, instance->settings.password);
			Serial.print("Connecting to ");
      		Serial.print(instance->settings.ssid);
			Serial.print(" Password: ");
      		Serial.println(instance->settings.password);
			unsigned long startAttemptTime = millis();
			const unsigned long timeout = 10000; // 10 seconds
			while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
			    delay(500);
			    Serial.print(".");
			}
			if (WiFi.status() == WL_CONNECTED) {
			    Serial.println("\nWiFi connected successfully!");
			    Serial.println("IP address: ");
			    Serial.println(WiFi.localIP());
          wifi_status = 1;
			} else {
			    Serial.println("\nFailed to connect to WiFi.");
			}
			break;
		}
		delay(10);
	}
	vTaskDelete(NULL);
}

void InitializeSettingsServer::start_monitor_thread() {
    xTaskCreatePinnedToCore(
        monitor_network_connection,   // Task function
        "monitor_task",               // Name of task
        4096,                         // Stack size of task
        this,                         // Pass the instance as the task parameter
        1,                            // Priority of the task
        &Task1,                       // Task handle to keep track of created task
        0                             // Core where the task should run
    );
}