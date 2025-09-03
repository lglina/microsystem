#ifndef AGAPE_MODEMS_ESP32_H
#define AGAPE_MODEMS_ESP32_H

#include <stdint.h>

#include <Arduino.h>
#include <elapsedMillis.h>
#include <WebSocketsClient.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#else
#include <WiFi.h>
#include <WiFiMulti.h>
#endif

#define FIELD_LENGTH 32
#define MAX_APS 20

namespace Agape
{

namespace Modems
{

class ESP32Modem
{
public:
    ESP32Modem();

    void run();

private:
    struct AccessPoint
    {
        char m_name[FIELD_LENGTH];
        char m_password[FIELD_LENGTH];
    };

    void handleATCommand();
    void handleData();
    
    void getConfigOptions();
    void setConfigOptions( const String& command );
    void getWiFiStatus();
    void connectWiFi();
    void dial( const String& command );
    
    bool connectWebSockets( const String& address, int port );

    void loadAccessPoints();
    void saveAccessPoints();

    bool addAccessPoint( const String& name, const String& password );
    bool deleteAccessPoint( const String& name );

    static void webSocketsEvent( WStype_t type, uint8_t* payload, size_t length );
    void _webSocketsEvent( WStype_t type, uint8_t* payload, size_t length );

    String hexToStr( const String& hexString );
    int hexToInt( char hex );

    static ESP32Modem* s_instance;

#ifdef ESP8266
    ESP8266WiFiMulti* m_wifiMulti;
#else
    WiFiMulti* m_wifiMulti;
#endif

    bool m_carrier;
    
    String m_apName;
    String m_apPassword;

    WebSocketsClient m_webSocketsClient;

    AccessPoint m_accessPoints[MAX_APS];
    int m_numAccessPoints;

    String m_addAccessPointName;

    bool m_scanPending;
};

} // namespace Modems

} // namespace Agape

#endif // AGAPE_MODEMS_ESP32_H
