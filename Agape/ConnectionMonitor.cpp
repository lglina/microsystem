#include "Lines/Line.h"
#include "Loggers/Logger.h"
#include "TupleRoutes/TupleRoute.h"
#include "UI/TabBar.h"
#include "UI/VRTime.h"
#include "Utils/base64/base64.h"
#include "ConfigurationStore.h"
#include "ConnectionMonitor.h"
#include "Runnable.h"
#include "String.h"
#include "StringConstants.h"
#include "TupleRouter.h"

#include <string.h>

namespace Agape
{

using namespace Linda2;

ConnectionMonitor::ConnectionMonitor( Line& line,
                                      ConfigurationStore& configurationStore,
                                      TupleRouter& tupleRouter,
                                      TupleRoute& tupleRoute,
                                      UI::TabBar& tabBar,
                                      UI::VRTime& vrTime ) :
  m_line( line ),
  m_configurationStore( configurationStore ),
  m_tupleRouter( tupleRouter ),
  m_tupleRoute( tupleRoute ),
  m_tabBar( tabBar ),
  m_vrTime( vrTime )
{
    m_tabBar.create( _Ready,
                     ::strlen( _Ready ),
                     UI::TabBar::left,
                     true, // true == visible
                     _Ready,
                     0x17 );
    m_tabBar.create( _Carrier,
                     ::strlen( _Carrier ),
                     UI::TabBar::left,
                     true, // true == visible
                     _Carrier,
                     0x17 );
    m_tabBar.create( _Secure,
                     ::strlen( _Secure ),
                     UI::TabBar::left,
                     true, // true == visible
                     _Secure,
                     0x48 );
}

void ConnectionMonitor::run()
{
    Line::LineStatus lineStatus( m_line.getLineStatus() );
    if( lineStatus != m_lineStatus )
    {
        if( !m_lineStatus.m_carrier && lineStatus.m_carrier )
        {
            onConnect();
        }
        else if( m_lineStatus.m_carrier && !lineStatus.m_carrier )
        {
            onDisconnect();
        }

        m_lineStatus = lineStatus;

        updateStatusBar();
    }

    // FIXME: Does this belong here?
    if( m_lineStatus.m_carrier )
    {
        m_tupleRouter.run();
    }
}

void ConnectionMonitor::onConnect()
{
    LOG_DEBUG( "Line connected." );
    if( m_line.requiresAuthentication() )
    {
        if( m_configurationStore.hasKey( _accountAuthKey ) && m_configurationStore.hasKey( _deviceAuthKey ) )
        {
            LOG_DEBUG( "Sending account and device auth keys" );
            Tuple keyTuple;
            TupleRouter::setSourceID( keyTuple, m_tupleRouter.myID() );
            TupleRouter::setTupleType( keyTuple, _Authenticate );

            // Base64 encode and set account auth key.
            String accountAuthKey = m_configurationStore.get( _accountAuthKey );
            String accountAuthKeyEncoded;
            accountAuthKeyEncoded.resize( Base64encode_len( accountAuthKey.length() ), '\0' );
            Base64encode( &accountAuthKeyEncoded[0], &accountAuthKey[0], accountAuthKey.length() );
            accountAuthKeyEncoded.resize( accountAuthKeyEncoded.length() - 1 );
            keyTuple[_accountAuthKey] = accountAuthKeyEncoded;

            // Base64 encode and set device auth key.
            String deviceAuthKey = m_configurationStore.get( _deviceAuthKey );
            String deviceAuthKeyEncoded;
            deviceAuthKeyEncoded.resize( Base64encode_len( deviceAuthKey.length() ), '\0' );
            Base64encode( &deviceAuthKeyEncoded[0], &deviceAuthKey[0], deviceAuthKey.length() );
            deviceAuthKeyEncoded.resize( deviceAuthKeyEncoded.length() - 1 );
            keyTuple[_deviceAuthKey] = deviceAuthKeyEncoded;
            
            m_tupleRouter.route( keyTuple );
        }
        else
        {
            LOG_DEBUG( "ERROR: No account and/or device auth key configured. Unable to authenticate with server. Connection will not work." );
        }
    }

    LOG_DEBUG( "Line connected. Setting tuple routing criteria." );
    // Render unto Caesar...
    TupleRoutingCriteria tupleRoutingCriteria;
    tupleRoutingCriteria.m_destinationIDs.push_back( new Value( m_tupleRouter.myID() ) );
    m_tupleRoute.sendAddRoutingCriteriaRequest( tupleRoutingCriteria );

    m_vrTime.doRegister();
}

void ConnectionMonitor::onDisconnect()
{
    // FIXME: What do we do here?
}

void ConnectionMonitor::updateStatusBar()
{
    int attributes;
    if( m_lineStatus.m_ready )
    {
        attributes = 0x1F;
    }
    else
    {
        attributes = 0x17;
    }
    m_tabBar.update( _Ready, _Ready, attributes );

    if( m_lineStatus.m_carrier )
    {
        attributes = 0x1F;
    }
    else
    {
        attributes = 0x17;
    }
    m_tabBar.update( _Carrier, _Carrier, attributes );

    if( m_lineStatus.m_ready && m_lineStatus.m_secure )
    {
        attributes = 0x2F;
    }
    else
    {
        attributes = 0x48;
    }
    m_tabBar.update( _Secure, _Secure, attributes );
}

} // namespace Agape
