#include "Collections.h"
#include "ConfigurationStore.h"
#include "Lines/Line.h"
#include "LineDrivers/LineDriver.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "DummyModemLine.h"
#include "Value.h"

namespace
{
    const char* _AccessPointNames( "Access Point Names" );
    const char* _AccessPointScan( "Access Point Scan" );
    const char* _AddAccessPointName( "Add Access Point Name" );
    const char* _AddAccessPointPassword( "Add Access Point Password" );
    const char* _DeleteAccessPointName( "Delete Access Point Name" );

    const char* _accessPointsAdded( "accessPointsAdded" );
    const char* _accessPointsAvailable( "accessPointsAvailable" );
    const char* _dummyModem( "dummyModem" );

    const int connectDelay( 1000 ); // ms.
    const int dialDelay( 3000 ); // ms.
} // Anonymous namespace

namespace Agape
{

namespace Lines
{

DummyModem::DummyModem( LineDriver& lineDriver,
                        ConfigurationStore& configurationStore,
                        Timers::Factory& timerFactory,
                        bool dialSetsLinkAddress ) :
  Line( lineDriver ),
  m_configurationStore( configurationStore ),
  m_timer( timerFactory.makeTimer() ),
  m_dialSetsLinkAddress( dialSetsLinkAddress ),
  m_state( disconnected ),
  m_accessPointsLoaded( false )
{
}

DummyModem::~DummyModem()
{
    delete( m_timer );
}

void DummyModem::run()
{
    m_lineDriver.run();

    switch( m_state )
    {
    case connecting:
        if( m_timer->ms() >= connectDelay )
        {
            // If dial sets the link address, show as line connected immediately
            // (we assume linkReady will be set by the line driver after
            // dialling), otherwise wait until linkReady.
            if( m_dialSetsLinkAddress || m_lineDriver.linkReady() )
            {
                LOG_DEBUG( "DummyModemLine: Link ready. Connecting->Connected" );
                m_state = connected;
            }
            else
            {
                // LineDriver didn't become ready.
                LOG_DEBUG( "DummyModemLine: Link didn't become ready. Connecting->Disconnected" );
                m_state = disconnected;
            }
        }
        break;
    case connected:
        if( !m_dialSetsLinkAddress && !m_lineDriver.linkReady() )
        {
            LOG_DEBUG( "DummyModemLine: Link not ready. Connected->Disconnected" );
            m_state = disconnected;
        }
        break;
    case dialling:
        if( m_timer->ms() >= dialDelay )
        {
            if( m_lineDriver.linkReady() )
            {
                LOG_DEBUG( "DummyModemLine: Dialling->Carrier" );
                m_state = carrier;
            }
            else
            {
                if( !m_lineDriver.error() )
                {
                    LOG_DEBUG( "DummyModemLine: Link not ready after dialling delay" );
                    LOG_DEBUG( "DummyModemLine: Dialling->Connected" );
                    m_state = connected;
                }
                else
                {
                    LOG_DEBUG( "DummyModemLine: Link in error after dialling delay" );
                    LOG_DEBUG( "DummyModemLine: Dialling->Disconnected" );
                    m_state = disconnected;
                }
            }
        }
        break;
    case carrier:
        if( !m_lineDriver.linkReady() || m_lineDriver.error() )
        {
            LOG_DEBUG( "DummyModemLine: Link not ready or in error. Carrier->Disconnected" );
            m_state = disconnected;
        }
        break;
    default:
        break;
    }

    m_lineStatus.m_ready = ( m_state == connected ) || ( m_state == dialling ) || ( m_state == carrier );
    m_lineStatus.m_ringing = false;
    m_lineStatus.m_carrier = ( m_state == carrier );
    m_lineStatus.m_secure = ( m_lineStatus.m_carrier && m_lineDriver.isSecure() );
}

Vector< Line::ConfigOption > DummyModem::getConfigOptions()
{
    // We defer loading from the configuration store (as opposed to doing this
    // in the constructor) as the configuration store may not be fully
    // initialised at the point that the line is constructed.
    if( !m_accessPointsLoaded ) loadAccessPoints();

    String accessPointsAvailable;
    {
    Vector< String >::iterator it( m_accessPointsAvailable.begin() );
    for( ; it != m_accessPointsAvailable.end(); ++it )
    {
        if( !accessPointsAvailable.empty() )
        {
            accessPointsAvailable += ";";
        }
        accessPointsAvailable += *it;
    }
    }

    String accessPointsAdded;
    {
    Vector< String >::iterator it( m_accessPointsAdded.begin() );
    for( ; it != m_accessPointsAdded.end(); ++it )
    {
        if( !accessPointsAdded.empty() )
        {
            accessPointsAdded += ";";
        }
        accessPointsAdded += *it;
    }
    }

    Vector< ConfigOption > configOptions;

    configOptions.push_back( ConfigOption( _AccessPointNames,
                                           Line::select,
                                           accessPointsAdded ) );
    configOptions.push_back( ConfigOption( _AccessPointScan,
                                           Line::select,
                                           accessPointsAvailable ) );
    configOptions.push_back( ConfigOption( _AddAccessPointName,
                                           Line::text,
                                           String() ) );
    configOptions.push_back( ConfigOption( _AddAccessPointPassword,
                                           Line::encodedText,
                                           String() ) );
    configOptions.push_back( ConfigOption( _DeleteAccessPointName,
                                           Line::text,
                                           String() ) );

    return configOptions;
}

void DummyModem::setConfigOption( const String& name, const String& value )
{
    if( name == _AddAccessPointName )
    {
        m_addName = value;
    }
    else if( name == _AddAccessPointPassword )
    {
        m_accessPointsAdded.push_back( m_addName );
    }
    else if( name == _DeleteAccessPointName )
    {
        Vector< String >::iterator it( m_accessPointsAdded.begin() );
        for( ; it != m_accessPointsAdded.end(); ++it )
        {
            if( *it == value )
            {
                m_accessPointsAdded.erase( it );
                break;
            }
        }
    }
}

void DummyModem::connect()
{
    m_timer->reset();
    m_state = connecting;
    LOG_DEBUG( "DummyModemLine: ->Connecting" );
}

void DummyModem::registerNumber( const String& number )
{
}

void DummyModem::dial( const String& number )
{
    LOG_DEBUG( "DummyModemLine: Dial" );
    if( m_state == connected )
    {
        LOG_DEBUG( "DummyModemLine: Connected->Dialling" );
        if( m_dialSetsLinkAddress )
        {
            m_lineDriver.setLinkAddress( number );
        }

        m_timer->reset();
        m_state = dialling;
    }
    else
    {
        LOG_DEBUG( "DummyModemLine: Dial called but line not connected!" );
    }
}

void DummyModem::answer()
{
}

void DummyModem::hangup()
{
    if( m_state == carrier )
    {
        m_state = connected;
    }
}

struct Line::LineStatus DummyModem::getLineStatus()
{
    return m_lineStatus;
}

void DummyModem::loadAccessPoints()
{
    if( m_configurationStore.hasKey( _dummyModem ) )
    {
        Value& config( m_configurationStore.get( _dummyModem ) );
        if( config.hasValue( _accessPointsAvailable ) )
        {
            Value& accessPointsAvailable( config[_accessPointsAvailable] );
            Vector< Value* >::const_iterator it( accessPointsAvailable.listBegin() );
            for( ; it != accessPointsAvailable.listEnd(); ++it )
            {
                m_accessPointsAvailable.push_back( **it );
            }
        }
        if( config.hasValue( _accessPointsAdded ) )
        {
            Value& accessPointsAdded( config[_accessPointsAdded] );
            Vector< Value* >::const_iterator it( accessPointsAdded.listBegin() );
            for( ; it != accessPointsAdded.listEnd(); ++it )
            {
                m_accessPointsAdded.push_back( **it );
            }
        }
    }
    else
    {
        m_accessPointsAvailable.push_back( "Ellingson Mineral Corporation" );
        m_accessPointsAvailable.push_back( "Cyberdyne Systems" );
        m_accessPointsAvailable.push_back( "AIMM Alderson" );
    }
}

} // namespace Lines

} // namespace Agape
