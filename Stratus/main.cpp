#include "Loggers/Logger.h"
#include "Loggers/StreamLogger.h"

#ifdef HYDRA
#include "WSHydraStratus.h"
#else
#include "WSRedisStratus.h"
#endif

int main( int argc, char** argv )
{
    Agape::Loggers::Stream streamLogger;
    Agape::Logger::setInstance( &streamLogger );

    LOG_DEBUG( "Stratus starting" );
    LOG_DEBUG( "(C) Lauren Glina 2019-2026" );

    Agape::Stratus::Stratus stratus( 8443 );
    stratus.run();
}
