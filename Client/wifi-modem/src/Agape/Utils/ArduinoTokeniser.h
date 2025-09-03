#ifndef AGAPE_UTILS_ARDUINO_TOKENISER_H
#define AGAPE_UTILS_ARDUINO_TOKENISER_H

#include <Arduino.h>

namespace Agape
{

class ArduinoTokeniser
{
public:
    ArduinoTokeniser( const String& string, char delim = '\n' );

    String token();
    String token( char delim );

private:
    const String& m_string;
    char m_delim;
    int m_from;
};

} // namespace Agape

#endif // AGAPE_UTILS_ARDUINO_TOKENISER_H
