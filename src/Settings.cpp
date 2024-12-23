#include "Settings.hpp"

void Settings::set_ssid(const char *ssid) { this->ssid = ssid; }
void Settings::set_password(const char *password) { this->password = password; }
Settings::Settings(const Settings &settings): ssid(settings.ssid), password(settings.password){}
Settings::Settings(const char *ssid, const char *password): ssid(ssid), password(password){}
Settings::Settings(){}