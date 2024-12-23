#include "Settings.hpp"
#include "string.h"
void Settings::set_ssid(const char* new_ssid) {
    if (ssid) {
        delete[] ssid; // Освобождаем старую память
    }
    ssid = new char[strlen(new_ssid) + 1];
    strcpy(ssid, new_ssid); // Копируем новое значение
}

void Settings::set_password(const char* new_password) {
    if (password) {
        delete[] password; // Освобождаем старую память
    }
    password = new char[strlen(new_password) + 1];
    strcpy(password, new_password); // Копируем новое значение
}


Settings::Settings(const Settings &settings): ssid(settings.ssid), password(settings.password){}
// Settings::Settings(const char *ssid, const char *password): ssid(ssid), password(password){}
Settings::Settings(){}