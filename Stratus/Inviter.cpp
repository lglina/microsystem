#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Authenticator.h"
#include "Inviter.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include <string>
#include <fstream>

#include <cpr/cpr.h>

namespace Agape
{

namespace Stratus
{

Inviter::Inviter( TupleRouter& tupleRouter, Authenticator& authenticator ) :
  m_tupleRouter( tupleRouter ),
  m_authenticator( authenticator ),
  Native( _InviteFriendServer )
{
    m_tupleRouter.registerActor( this );

    std::ifstream secretFile( "inviter-secret" );
    std::getline( secretFile, m_secret );

    if( !secretFile.is_open() || secretFile.fail() )
    {
        LOG_DEBUG( "WARNING: Failed to read inviter secret from 'inviter-secret' file. Invitations will not work." );
    }

    secretFile.close();
}

Inviter::~Inviter()
{
    m_tupleRouter.deregisterActor( this );
}

bool Inviter::accept( Tuple& tuple )
{
    bool handled( false );

    String tupleType( TupleRouter::tupleType( tuple ) );
    if( tupleType == _InviteFriendRequest )
    {
        LOG_DEBUG( "Inviter: Received InviteFriendRequest" );

        bool success( false );

        String name = tuple[_name];
        String friendsName = tuple[_friendsName];
        String friendsEmail = tuple[_friendsEmail];
        String sealingKey = tuple[_sealingKey];
        String sealedWorldKey = tuple[_sealedWorldKey];

        if( !name.empty() &&
            !friendsName.empty() &&
            !friendsEmail.empty() &&
            !sealingKey.empty() &&
            !sealedWorldKey.empty() )
        {
            // Simply forward the tuple contents through to Denarii as a JSON
            // payload. The rails app will save the sealed world key to
            // MongoDB, and send the sealing key (and invite link, identified
            // by sealing key SHA256 hash) by email to the invitee.
            LiteStream jsonRequest;
            jsonRequest << "{\"invitation\": {"
                        << "\"name\": \"" << name << "\", "
                        << "\"invitedBy\": \"" << m_authenticator.accountAuthKeyHash() << "\", "
                        << "\"friendsName\": \"" << friendsName << "\", "
                        << "\"friendsEmail\": \"" << friendsEmail << "\", "
                        << "\"sealingKey\": \"" << sealingKey << "\", "
                        << "\"sealedWorldKey\": \"" << sealedWorldKey << "\""
                        << "} }";
            LOG_DEBUG( "Inviter: Sending invitation" );
            cpr::Response response( cpr::Post( cpr::Url{ "https://glina.com.au/invitations" },
                                               cpr::Body{ jsonRequest.str() },
                                               cpr::Header{ { "Content-Type", "application/json" } },
                                               cpr::Header{ { "Authorization", std::string( m_secret.c_str() ) } } ) );

            // We could read the JSON response here, but adequate to just look at the HTTP status.
            if( response.status_code == 200 )
            {
                LOG_DEBUG( "Inviter: Invitation successfully queued on webserver" );
                success = true;
            }
            else if( response.status_code == 0 )
            {
                LiteStream errorStream;
                errorStream << "Inviter: Request error: " << response.error.message.c_str();
                LOG_DEBUG( errorStream.str() );
            }
            else
            {
                LiteStream errorStream;
                errorStream << "Inviter: Response status not OK. Got code " << response.status_code;
                LOG_DEBUG( errorStream.str() );
            }
        }

        Tuple response;
        TupleRouter::setSourceActor( response, _InviteFriendServer );
        TupleRouter::setSourceID( response, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( response, _InviteFriendResponse );
        response[_success] = success ? 1 : 0;

        LOG_DEBUG( "Inviter: Sending InviteFriendResponse" );
        m_tupleRouter.route( response );

        handled = true;
    }

    return handled;
}

} // namespace Stratus

} // namespace Agape
