#include "ESP32Modem.h"

#include <Arduino.h>

Agape::Modems::ESP32Modem* g_modem;

void setup() {
    setCpuFrequencyMhz( 160 );
    //esp_log_level_set( "*", ESP_LOG_INFO );
    g_modem = new Agape::Modems::ESP32Modem();
}

void loop() {
    g_modem->run();
}
