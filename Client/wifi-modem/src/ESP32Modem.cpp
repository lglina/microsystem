#include "Agape/Utils/ArduinoTokeniser.h"
#include "ESP32Modem.h"

#include <algorithm>
#include <Arduino.h>
#include <EEPROM.h>
#include <elapsedMillis.h>
#include <HexDump.h>
#include <string.h>
#include <stdint.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#else
#include <WiFi.h>
#include <WiFiMulti.h>
#endif

namespace
{
    const char* _AccessPointNames( "Access Point Names" );
    const char* _AccessPointScan( "Access Point Scan" );
    const char* _AddAccessPointName( "Add Access Point Name" );
    const char* _AddAccessPointPassword( "Add Access Point Password" );
    const char* _DeleteAccessPointName( "Delete Access Point Name" );

    const int connectTimeout( 10000 ); // ms.
    const int defaultPort( 8443 );

    const uint32_t pingInterval( 30000 ); // ms betewen WS ping.
    const uint32_t pongTimeout( 10000 ); // ms to wait for WS pong.
    const uint32_t disconnectTimeoutCount( 3 ); // No. of missed pongs before disconnect.
    const uint32_t reconnectInterval( 10000 ); // ms to wait between reconnect attempts.
} // Anonymous namespace

namespace Agape
{

namespace Modems
{

ESP32Modem* ESP32Modem::s_instance( nullptr );

ESP32Modem::ESP32Modem() :
  m_wifiMulti( nullptr ),
  m_carrier( false ),
  m_numAccessPoints( 0 ),
  m_scanPending( false )
{
    s_instance = this;

    pinMode( 0, OUTPUT );   //  /DCD
    pinMode( 1, OUTPUT );    //  /RI
    pinMode( 10, INPUT );   //  /FLIGHT
    pinMode( 3, OUTPUT );   //  NC
    pinMode( 18, OUTPUT );  //  NC
    pinMode( 19, OUTPUT );  //  NC
    pinMode( 5, INPUT );    //  /RTS
    pinMode( 4, OUTPUT );   //  /CTS

    digitalWrite( 0, 1 ); // No carrier.
    digitalWrite( 1, 1 ); // Not ringing.

    // Serial: Debug.
    // Serial1: DTE comms.

    Serial1.setRxBufferSize( 4096 );
    Serial1.setTxBufferSize( 4096 );
    Serial.begin( 115200 );

    Serial1.begin( 800000, SERIAL_8N1, 6, 7 );
    Serial1.setPins( -1, -1, 5, 4 );
    Serial1.setHwFlowCtrlMode();

    Serial1.setTimeout(10);

    Serial.println( "Micro System WiFi Modem v1.0" );
    Serial.println( "(C) Lauren Glina 2023-2025" );

    WiFi.mode( WIFI_STA );
    WiFi.setMinSecurity( WIFI_AUTH_OPEN );
    WiFi.disconnect();

    EEPROM.begin( 1024 );
    loadAccessPoints();
}

void ESP32Modem::run()
{
    m_webSocketsClient.loop();
    if( !m_carrier )
    {
        handleATCommand();
    }
    else
    {
        handleData();
    }
}

void ESP32Modem::handleATCommand()
{
    String command( Serial1.readStringUntil( '\n' ) );
    if( !command.isEmpty() )
    {
        Serial.println( "Received AT command: " + command );
    }

    if( command == "AT+GCONF?\r" )
    {
        getConfigOptions();
    }
    else if( command.startsWith( "AT+GCONF=" ) )
    {
        setConfigOptions( command );
    }
    else if( command == "AT+XCONN?\r" )
    {
        getWiFiStatus();
    }
    else if( command == "AT+XCONN\r" )
    {
        connectWiFi();
    }
    else if( command.startsWith( "ATD" ) )
    {
        dial( command );
    }
    else if( !command.isEmpty() )
    {
        Serial1.println( "ERROR" );
    }
}

void ESP32Modem::handleData()
{
    if( Serial1.available() )
    {
        delay( 1 ); // FIXME: Hack - wait a little for the RX buffer to fill up before reading it...
        //Serial.println( "Receiving from serial" );
        // Might be more that 1024B to receive, especially if Websockets
        // was slow sending last time, but let's stay under MTU.
        char buffer[1024];
        //size_t bytesRead( Serial1.read( buffer, 1024 ) );
        size_t bytesRead( Serial1.readBytes( buffer, 1024 ) );

        //HexDump( Serial, buffer, bytesRead );

        if( bytesRead > 0 )
        {
            Serial.printf( "Sending %d bytes to websockets\r\n", bytesRead );
            m_webSocketsClient.sendBIN( (uint8_t*)buffer, bytesRead );
            //Serial.println( "Sent." );
        }
    }

    if( WiFi.status() != WL_CONNECTED )
    {
        m_carrier = false;
        digitalWrite( 0, 1 );
        Serial.println( "WiFi disconnected" );
        Serial1.println( "NO CARRIER" );
    }
}

void ESP32Modem::getConfigOptions()
{
    Serial.println( "Getting config options" );

    // Added access points.
    Serial1.print( _AccessPointNames );
    Serial1.print( ",select," );
    for( int i = 0; i < m_numAccessPoints; ++i )
    {
        if( i != 0 )
        {
            Serial1.print( ';' );
        }

        Serial1.print( m_accessPoints[i].m_name );
    }
    Serial1.print( "\r\n" );

    if( m_scanPending )
    {
        // Scanned access points
        int numAPs( WiFi.scanNetworks() );
        std::pair<int, int> rssis[numAPs];
        for( int i = 0; i < numAPs; ++i )
        {
            rssis[i] = std::make_pair( WiFi.RSSI( i ), i );
        }
        // Sort by strongest signal strength.
        std::sort( rssis, rssis + numAPs, std::greater<std::pair<int, int>>() );

        Serial1.print( _AccessPointScan );
        Serial1.print( ",select," );
        for( int i = 0; i < numAPs; ++i )
        {
            if( i != 0 )
            {
                Serial1.print( ';' );
            }
            Serial1.print( WiFi.SSID( rssis[i].second ) );
        }
        Serial1.print( "\r\n" );

        m_scanPending = false;
    }

    Serial1.println( "END" );

    Serial.println( "Done getting config options" );
}

void ESP32Modem::setConfigOptions( const String& command )
{
    Serial.println( "Setting config option" );

    String options( command.substring( 9, command.length() - 1 ) ); // Cut off command before '=' and trailing \r.

    String addAccessPointPassword;
    String deleteAccessPointName;

    bool success( false );

    ArduinoTokeniser tokeniser( options, ',' );
    String token( tokeniser.token() );
    while( token != "" )
    {
        ArduinoTokeniser optionTokeniser( token, '=' );
        String name( optionTokeniser.token() );
        String value( optionTokeniser.token() );

        if( name == _AddAccessPointName )
        {
            m_addAccessPointName = value;
            success = true;
        }
        else if( name == _AddAccessPointPassword )
        {
            addAccessPointPassword = hexToStr( value );
        }
        else if( name == _DeleteAccessPointName )
        {
            deleteAccessPointName = value;
        }
        else if( name == _AccessPointScan )
        {
            m_scanPending = true;
            success = true;
        }

        token = tokeniser.token();
    }

    if( !m_addAccessPointName.isEmpty() && !addAccessPointPassword.isEmpty() )
    {
        success = addAccessPoint( m_addAccessPointName, addAccessPointPassword );
        saveAccessPoints();
        m_addAccessPointName.clear();
    }

    if( !deleteAccessPointName.isEmpty() )
    {
        success = deleteAccessPoint( deleteAccessPointName );
        saveAccessPoints();
    }

    if( success )
    {
        Serial1.println( "OK" );
    }
    else
    {
        Serial1.println( "ERROR" );
    }

    Serial.println( "Done setting config option" );
}

void ESP32Modem::getWiFiStatus()
{
    if( WiFi.status() == WL_CONNECTED )
    {
        Serial.println( "Status: Connected" );
        Serial1.println( "CONNECTED" );
    }
    else if( WiFi.status() == WL_IDLE_STATUS )
    {
        Serial.println( "Status: Connecting" );
        Serial1.println( "CONNECTING" );
    }
    else
    {
        Serial.println( "Status: Disconnected" );
        Serial1.println( "DISCONNECTED" );
    }
}

void ESP32Modem::connectWiFi()
{
    Serial.println( "Connecting WiFi" );

    if( WiFi.status() != WL_CONNECTED )
    {
        if( m_wifiMulti != nullptr )
        {
            delete( m_wifiMulti );
        }
#ifdef ESP8266
        m_wifiMulti = new ESP8266WiFiMulti;
#else
        m_wifiMulti = new WiFiMulti;
#endif
        for( int i = 0; i < m_numAccessPoints; ++i )
        {
            Serial.println( "Add AP:" );
            Serial.println( m_accessPoints[i].m_name );
            Serial.println( m_accessPoints[i].m_password );
            m_wifiMulti->addAP( m_accessPoints[i].m_name, m_accessPoints[i].m_password );
        }

        m_wifiMulti->run();

        if( WiFi.status() == WL_CONNECTED )
        {
            Serial.println( "WiFi Connected" );
            Serial1.println( "OK" );
            return;
        }
    }

    Serial.println( "WiFi Connect error" );
    Serial1.println( "ERROR" );
}

void ESP32Modem::dial( const String& command )
{
    Serial.println( "Connecting websockets" );

    String address( command.substring( 3, command.length() - 1 ) ); // From ATD to trailing \r
    int hostIndex( address.indexOf( "//" ) );
    if( hostIndex != -1 )
    {
        int portIndex = address.indexOf( ":", hostIndex + 2 );
        int port;
        if( portIndex != -1 )
        {
            String portstr( address.substring( portIndex + 1, address.length() - 1 ) ); // Assume there is a trailing "/".
            port = atoi( portstr.c_str() );
            address = address.substring( hostIndex + 2, portIndex );
        }
        else
        {
            port = defaultPort;
            address = address.substring( hostIndex + 2 );
        }

        Serial.printf( "Address: %s Port: %d", address.c_str(), port );

        if( connectWebSockets( address, port ) )
        {
            Serial.println( "Websockets connected" );
            Serial1.println( "OK" );
            return;
        }
    }
    
    Serial.println( "Websockets connect error" );
    Serial1.println( "ERROR" );
}

bool ESP32Modem::connectWebSockets( const String& address, int port )
{
    m_webSocketsClient.enableHeartbeat( pingInterval, pongTimeout, disconnectTimeoutCount );
    m_webSocketsClient.setReconnectInterval( reconnectInterval );
    m_webSocketsClient.beginSSL( address.c_str(), port, "/", "", "Linda2" );
    m_webSocketsClient.onEvent( ESP32Modem::webSocketsEvent );

    elapsedMillis millis;
    while( !m_carrier &&
           ( millis < connectTimeout ) )
    {
        m_webSocketsClient.loop();
    }

    if( millis < connectTimeout )
    {
        return true;
    }
    else
    {
        // FIXME: WebSocketsClient disconnect() doesn't stop auto-reconnection
        // attempts. Destruct to reset back to disconnected state.
        m_webSocketsClient = WebSocketsClient();
        return false;
    }
}

void ESP32Modem::loadAccessPoints()
{
    Serial.println( "Loading access points" );
    m_numAccessPoints = EEPROM.read( 0 );
    if( m_numAccessPoints <= MAX_APS )
    {
        for( int i = 0; i < m_numAccessPoints; ++i )
        {
            int startOffset( 1 + ( i * FIELD_LENGTH * 2 ) );
            for( int j = 0; j < FIELD_LENGTH; ++j )
            {
                m_accessPoints[i].m_name[j] = EEPROM.read( startOffset + j );
            }
            for( int j = 0; j < FIELD_LENGTH; ++j )
            {
                m_accessPoints[i].m_password[j] = EEPROM.read( startOffset + FIELD_LENGTH + j );
            }
        }

        Serial.printf( "%d access point(s) loaded successfully\r\n", m_numAccessPoints );
    }
    else
    {
        m_numAccessPoints = 0;
        Serial.println( "EEPROM memory invalid. No access points loaded." );
    }
}

void ESP32Modem::saveAccessPoints()
{
    Serial.println( "Saving access points" );
    EEPROM.write( 0, 0xFF );
    for( int i = 0; i < m_numAccessPoints; ++i )
    {
        int startOffset( 1 + ( i * FIELD_LENGTH * 2 ) );
        for( int j = 0; j < FIELD_LENGTH; ++j )
        {
            EEPROM.write( startOffset + j, m_accessPoints[i].m_name[j] );
        }
        for( int j = 0; j < FIELD_LENGTH; ++j )
        {
            EEPROM.write( startOffset + FIELD_LENGTH + j, m_accessPoints[i].m_password[j] );
        }
    }
    EEPROM.write( 0, m_numAccessPoints );
    EEPROM.commit();
    Serial.printf( "%d access point(s) saved successfully\r\n", m_numAccessPoints );
}

bool ESP32Modem::addAccessPoint( const String& name, const String& password )
{
    Serial.print( "Adding access point " );
    Serial.println( name );
    if( m_numAccessPoints < MAX_APS )
    {
        ::strncpy( m_accessPoints[m_numAccessPoints].m_name, name.c_str(), FIELD_LENGTH );
        ::strncpy( m_accessPoints[m_numAccessPoints].m_password, password.c_str(), FIELD_LENGTH );
        ++m_numAccessPoints;
        
        return true;
    }

    return false;
}

bool ESP32Modem::deleteAccessPoint( const String& name )
{
    Serial.print( "Deleting access point " );
    Serial.println( name );
    bool success( false );
    for( int i = 0; ( i < m_numAccessPoints ) && !success; ++i )
    {
        if( ::strncmp( m_accessPoints[i].m_name, name.c_str(), FIELD_LENGTH ) == 0 )
        {
            for( int j = i + 1; j < m_numAccessPoints; ++j )
            {
                ::strncpy( m_accessPoints[i - 1].m_name, m_accessPoints[i].m_name, FIELD_LENGTH );
                ::strncpy( m_accessPoints[i - 1].m_password, m_accessPoints[i].m_password, FIELD_LENGTH );
            }
            --m_numAccessPoints;
            success = true;
        }
    }

    return success;
}

void ESP32Modem::webSocketsEvent( WStype_t type, uint8_t* payload, size_t length )
{
    s_instance->_webSocketsEvent( type, payload, length );
}

void ESP32Modem::_webSocketsEvent( WStype_t type, uint8_t* payload, size_t length )
{
    switch( type )
    {
    case WStype_DISCONNECTED:
        if( m_carrier )
        {
            m_carrier = false;
            digitalWrite( 0, 1 );
            Serial.println( "WS Ev: Disconnected" );
            Serial1.println( "NO CARRIER" );
        }
        break;
    case WStype_ERROR:
        if( m_carrier )
        {
            m_carrier = false;
            digitalWrite( 0, 1 );
            Serial.println( "WS Ev: Error" );
            Serial1.println( "NO CARRIER" );
        }
        break;
    case WStype_CONNECTED:
        m_carrier = true;
        digitalWrite( 0, 0 );
        Serial.println( "WS Ev: Connected" );
        break;
    case WStype_BIN:
        //Serial.println( "WS Ev: Bin" );
        Serial.printf( "Received %d bytes from websockets\r\n", length );
        //HexDump( Serial, payload, length );
        Serial1.write( payload, length );
        //Serial.println( "Sent to serial" );
        break;
    default:
        break;
    }
}

String ESP32Modem::hexToStr( const String& hexString )
{
    String s;
    for( int i = 0; i < hexString.length() / 2; ++i )
    {
        char highNybble( hexString[i*2] );
        char lowNybble( hexString[(i*2)+1] );
        int charVal( ( hexToInt( highNybble ) << 4 ) + hexToInt( lowNybble ) );
        s += (char)charVal;
    }
    return s;
}

int ESP32Modem::hexToInt( char hex )
{
    if( hex >= '0' && hex <= '9' )
    {
        return hex - '0';
    }
    else if( hex >= 'a' && hex <= 'f' )
    {
        return 10 + ( hex - 'a' );
    }
    else if( hex >= 'A' && hex <= 'F' )
    {
        return 10 + ( hex - 'A' );
    }
    return 0;
}

} // namespace Modems

} // namespace Agape
