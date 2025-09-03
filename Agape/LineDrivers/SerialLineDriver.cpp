#include "LineDrivers/LineDriver.h"
#include "LineDrivers/SerialLineDriver.h"
#include "Utils/StrToHex.h"
#include "String.h"

#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <unistd.h>

namespace Agape
{

namespace LineDrivers
{

Serial::Serial( const String& device, int baudRate ) :
  m_device( device ),
  m_baudRate( baudRate ),
  m_port( 0 ),
  m_isOpen( false )
{
}

int Serial::open()
{
    m_port = ::open( m_device.c_str(), O_RDWR | O_NONBLOCK );

    if( m_port < 0 )
    {
        std::cerr << "Unable to open serial port " << m_device << std::endl;
        throw std::exception();
    }

    struct termios orig_term_settings; // Saved terminal settings
    struct termios new_term_settings; // Current terminal settings

    // Save the default parameters
    int rc = ::tcgetattr( m_port, &orig_term_settings );

    // Set raw mode on the slave side of the PTY
    new_term_settings = orig_term_settings;
    ::cfmakeraw( &new_term_settings );
    new_term_settings.c_cflag &= ~PARENB;
    new_term_settings.c_cflag &= ~CSTOPB;
    new_term_settings.c_cflag |= CS8;

    // Enable hardware flow control
    new_term_settings.c_cflag |= CRTSCTS;

    speed_t baudRate = B57600;
    if( m_baudRate == 57600 )
    {
        baudRate = B57600;
    }
    else if( m_baudRate == 115200 )
    {
        baudRate = B115200;
    }
    else if( m_baudRate == 460800 )
    {
        baudRate = B460800;
    }
    // FIXME: Add others.

    ::cfsetspeed( &new_term_settings, baudRate );

    ::tcsetattr( m_port, TCSANOW, &new_term_settings );
    std::cerr << "Serial port opened successfully" << std::endl;

    m_isOpen = true;

    return 0;
}

int Serial::read( char* data, int len )
{
    if( m_isOpen )
    {
        int ret = ::read( m_port, data, len );
        if( ret >= 0 )
        {
            if( ret > 0 )
            {
                std::cerr << "Serial read " << len << ": " << String( data, len ) << std::endl;
                hexDump( data, len );
            }
            return ret;
        }
    }

    return 0;
}

int Serial::write( const char* data, int len )
{
    if( m_isOpen )
    {
        std::cerr << "Serial write " << len << ": " << String( data, len ) << std::endl;
        hexDump( data, len );
        int ret = ::write( m_port, data, len );
        if( ret >= 0 )
        {
            return ret;
        }
    }

    return 0;
}

} // namespace LineDrivers

} // namespace Agape
