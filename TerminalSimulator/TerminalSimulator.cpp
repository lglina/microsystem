#include "Loggers/Logger.h"
#include "Loggers/StreamLogger.h"
#include "Chooser.h"
#include "ClientBuilder.h"
#include "Collections.h"
#include "TerminalSimulator.h"
#include "SimulatedOfflineClientBuilder.h"
#include "SimulatedOnlineClientBuilder.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <string>

using namespace Agape;

namespace Agape
{

TerminalSimulator::TerminalSimulator( Chooser& chooser ) :
  m_chooser( chooser ),
  m_currentBuilder( nullptr ),
  m_client( nullptr )
{
}

TerminalSimulator::~TerminalSimulator()
{
    LOG_DEBUG( "TerminalSimulator: Destructing" );
}

void TerminalSimulator::run()
{
    if( m_chooser.changed() )
    {
        m_currentBuilder->unbuild();
        m_client = nullptr;
        m_chooser.resetChanged();
    }

    if( !m_client )
    {
      m_currentBuilder = m_chooser.currentChoice();
      m_client = m_currentBuilder->build( m_chooser );
    }

    m_client->run();
}

} // namespace Agape

int main( int argc, char** argv )
{
    Logger::setInstance( new Loggers::Stream );

    LOG_DEBUG( "TerminalSimulator starting" );
    LOG_DEBUG( "(C) Lauren Glina 2019-2025" );

    QApplication app( argc, argv );
    
    Chooser chooser;
    chooser.addChoice( new ClientBuilders::SimulatedOnline, "Online" );
    chooser.addChoice( new ClientBuilders::SimulatedOffline, "Offline" );

    if( argc == 2 ) chooser.setCurrentChoice( argv[1] ); // else default is Online.

    Agape::TerminalSimulator ts( chooser );

    QTimer timer;
    QObject::connect( &timer, &QTimer::timeout, &ts, &Agape::TerminalSimulator::run );
    timer.start( 10 );

    int retcode( app.exec() );

    return retcode;

    // The client will be destructed by the builder when the
    // builder object is destructed.
}
