#include "Loggers/Logger.h"
#include "Loggers/StreamLogger.h"
#include "Stratus.h"

int main( int argc, char** argv )
{
    Agape::Loggers::Stream streamLogger;
    Agape::Logger::setInstance( &streamLogger );

    LOG_DEBUG( "Stratus starting" );
    LOG_DEBUG( "(C) Lauren Glina 2019-2025" );

    Agape::Stratus::Stratus stratus( 8443 );
    stratus.run();
}
