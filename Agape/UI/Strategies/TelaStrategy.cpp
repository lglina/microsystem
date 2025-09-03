#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "UI/Dialogue.h"
#include "Utils/base64/base64.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "WorldLoaders/WorldLoader.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "Collections.h"
#include "ConfigurationStore.h"
#include "KeyUtilities.h"
#include "Phonebook.h"
#include "String.h"
#include "StringConstants.h"
#include "TelaStrategy.h"
#include "Value.h"
#include "Worldbook.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace Agape::World;

namespace Agape
{

namespace UI
{

namespace Strategies
{

Tela::Tela( InputDevice& inputDevice,
            WorldLoaders::Factory& worldLoaderFactory,
            ConfigurationStore& configurationStore,
            World::Utilities& worldUtilities,
            KeyUtilities& keyUtilities,
            Agape::Phonebook& phonebook,
            Agape::Worldbook& worldbook,
            Dialogue& dialogue,
            const String& defaultPhoneName,
            const String& defaultPhoneNumber ) :
  m_inputDevice( inputDevice ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_configurationStore( configurationStore ),
  m_worldUtilities( worldUtilities ),
  m_keyUtilities( keyUtilities ),
  m_phonebook( phonebook ),
  m_worldbook( worldbook ),
  m_dialogue( dialogue ),
  m_defaultPhoneName( defaultPhoneName ),
  m_defaultPhoneNumber( defaultPhoneNumber ),
  m_state( none ),
  m_completed( false ),
  m_calling( false )
{
}

void Tela::enter( const Value& parameters )
{
    m_completed = false;

    initPhonebook();
    setKeys();
    getInviteeKey();

    if( m_joinKey.empty() )
    {
        LOG_DEBUG( "Join key empty. Setting OOBE complete." );
        // We're not a newly invited user, so set OOBE to completed.
        Value& oobeValue( m_configurationStore.get( _oobe ) );
        oobeValue[_state] = _completed;
        m_configurationStore.save();
    }
    else
    {
        LOG_DEBUG( "Join key is " + m_joinKey + ". Enabling OOBE." );
    }

    m_state = connect;
    m_nextStrategy = "connect";
    m_calling = true;
}

void Tela::returnTo( const Value& parameters )
{
    m_calling = false;
    m_callingParameters = Value();

    if( m_state == connect ) // <- Returning from
    {
        if( (int)parameters[_success] == 1 )
        {
#ifdef __EMSCRIPTEN__
            if( m_joinKey.empty() )
            {
                drawLoadPending();
                if( _load() )
                {
                    hideDialogue();
                    m_completed = true;
                }
                else
                {
                    drawLoadError();
                    m_state = loadError;
                }
            }
            else
            {
                // We're a newly invited user. Go ahead and join to world
                // and initiate onboarding.
                m_state = chooseAvatar;
                m_nextStrategy = "chooseAvatar";
                m_calling = true;
                m_callingParameters[_title] = _welcome;
            }
#else
            drawLoadPending();
            if( _load() )
            {
                hideDialogue();
                m_completed = true;
            }
            else
            {
                drawLoadError();
                m_state = loadError;
            }
#endif
        }
        else
        {
            drawLoadError();
            m_state = loadError;
        }
    }
    else if( m_state == chooseAvatar ) // <- Returning from
    {
        if( (int)parameters[_success] == 1 )
        {
            m_state = join;
            m_nextStrategy = "joinWorld";
            m_calling = true;
            m_callingParameters[_title] = _welcome;
            m_callingParameters[_user] = parameters;
            m_callingParameters[_worldKey] = m_joinKey;
            if( m_configurationStore.hasKey( _accountSubKey ) )
            {
                m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
            }
        }
        else
        {
            m_completed = true;
        }
    }
    else if( m_state == join ) // <- Returning from
    {
        if( (int)parameters[_success] == 1 )
        {
            Metadata metadata( Metadata::fromValue( parameters[_metadata], true ) ); // true = deserialise keys.
            User user( User::fromValue( parameters[_user] ) );

            bool makeDefault( !m_worldbook.hasDefaultWorldID() );
            m_worldbook.add( metadata, makeDefault );
            m_worldbook.setUserForWorld( metadata.m_worldID, user );

            // Initiate onboarding to join the user to the learning
            // world and add teleports.
            Value& oobeValue( m_configurationStore.get( _oobe ) );
            oobeValue[_state] = _teleportDemo;
            m_configurationStore.save();
        }
        m_completed = true;
    }
}

bool Tela::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        return true;
    }

    return false;
}

bool Tela::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
#ifdef __EMSCRIPTEN__
        eraseInviteeKey(); // To protect key, and prevent re-join on page reload.
#endif
        return true;
    }

    return false;
}

void Tela::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }
    
        switch( m_state )
        {
        case loadError:
            if( c == '\n' )
            {
                hideDialogue();
#ifdef __EMSCRIPTEN__
                emscripten_run_script_string( "location.reload();" );
#endif
                m_completed = true;
            }
            break;
        default:
            break;
        }
    }
}

void Tela::drawLoadPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Loading user worlds..." );
}

void Tela::drawLoadError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Worlds Not Loaded" );
#ifdef __EMSCRIPTEN__
    m_dialogue.drawMessage( "There was an error loading your worlds. Hit \x1b[97;41mRet\x1b[37;41m\
                             to reload the page to try again." );
#else
    m_dialogue.drawMessage( "There was an error loading your worlds. Hit \x1b[97;41mRet\x1b[37;41m\
                             to go back." );
#endif
}

void Tela::hideDialogue()
{
    m_dialogue.hide();
}

void Tela::initPhonebook()
{
    // Add Tela server as default. Overwrites number if entry already exists.
    m_phonebook.add( m_defaultPhoneName, m_defaultPhoneNumber, true ); // true = set default.
}

bool Tela::setKeys()
{
    // Get accountSubKey and accountAuthKey from browser storage with
    // Emscripten, and get deviceAuthKey from DOM, and set
    // in ConfigurationStore.
#ifdef __EMSCRIPTEN__
    {
    LiteStream stream;
    stream << "localStorage.getItem(\"" << _accountSubKey << "\");";
    String accountSubKeyEncoded( emscripten_run_script_string( stream.str().c_str() ) );
    LOG_DEBUG( "Account sub key: " + accountSubKeyEncoded );

    String accountSubKey( Base64decode_len( accountSubKeyEncoded.c_str() ), '\0' );
    accountSubKey.resize( Base64decode( &accountSubKey[0], accountSubKeyEncoded.c_str() ) );

    m_configurationStore.get( _accountSubKey ) = accountSubKey;
    }

    {
    LiteStream stream;
    stream << "localStorage.getItem(\"" << _accountAuthKey << "\");";
    String accountAuthKeyEncoded( emscripten_run_script_string( stream.str().c_str() ) );
    LOG_DEBUG( "Account auth key: " + accountAuthKeyEncoded );

    String accountAuthKey( Base64decode_len( accountAuthKeyEncoded.c_str() ), '\0' );
    accountAuthKey.resize( Base64decode( &accountAuthKey[0], accountAuthKeyEncoded.c_str() ) );

    m_configurationStore.get( _accountAuthKey ) = accountAuthKey;
    }

    {
    LiteStream stream;
    stream << "document.querySelector(\"#tela\").dataset.telaDeviceAuthKey;";
    String deviceAuthKeyEncoded( emscripten_run_script_string( stream.str().c_str() ) );
    LOG_DEBUG( "Device auth key: " + deviceAuthKeyEncoded );

    String deviceAuthKey( Base64decode_len( deviceAuthKeyEncoded.c_str() ), '\0' );
    deviceAuthKey.resize( Base64decode( &deviceAuthKey[0], deviceAuthKeyEncoded.c_str() ) );

    m_configurationStore.get( _deviceAuthKey ) = deviceAuthKey;
    }
#endif

    return true;
}

bool Tela::_load()
{
    bool success( true );

    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Vector< Metadata > joinedWorlds;
    String reason;
    success = worldLoader->loadJoinedWorlds( joinedWorlds, true, reason ); // true = load for all devices in account

    String accountSubKey;
    if( m_configurationStore.hasKey( _accountSubKey ) )
    {
        accountSubKey = m_configurationStore.get( _accountSubKey );
    }

    if( success )
    {
        Vector< Metadata >::iterator it( joinedWorlds.begin() );
        for( ; it != joinedWorlds.end(); ++it )
        {
            // Add world. Overwrites metadata if it already exists.
            // FIXME: The last added entry will be set to the default. How
            // do we allow web users to have a persisted default? Store
            // in a cookie and load here?

            Metadata& encryptedMetadata( *it );
            Metadata plainMetadata;
            encryptedMetadata.m_worldKey.resize( m_worldUtilities.worldKeySize(), '\0' );
            m_worldUtilities.decryptPrivateKey( encryptedMetadata, &accountSubKey[0], &encryptedMetadata.m_worldKey[0] );
            m_worldUtilities.decryptMetadata( &encryptedMetadata.m_worldKey[0], encryptedMetadata, plainMetadata );

            m_worldbook.add( plainMetadata, ( plainMetadata.m_worldID != _learningWorldID ) ); // Don't set default for learning world.
            if( !plainMetadata.m_users.empty() ) m_worldbook.setUserForWorld( plainMetadata.m_worldID, plainMetadata.m_users.front() ); // Users should never be empty.
        }
    }

    delete( worldLoader );
    return success;
}

void Tela::getInviteeKey()
{
#ifdef __EMSCRIPTEN__
    String sealedWorldKeyEncoded;
    String sealingKeyEncoded;

    {
    LiteStream stream;
    stream << "sessionStorage.getItem(\"" << _sealedWorldKey << "\");";
    sealedWorldKeyEncoded = emscripten_run_script_string( stream.str().c_str() );
    LOG_DEBUG( "Sealed world key: " + sealedWorldKeyEncoded );
    }

    {
    LiteStream stream;
    stream << "sessionStorage.getItem(\"" << _sealingKey << "\");";
    sealingKeyEncoded = emscripten_run_script_string( stream.str().c_str() );
    LOG_DEBUG( "Sealing key: " + sealingKeyEncoded );
    }

    if( !sealedWorldKeyEncoded.empty() && !sealingKeyEncoded.empty() )
    {
        String sealedWorldKey( Base64decode_len( sealedWorldKeyEncoded.c_str() ), '\0' );
        sealedWorldKey.resize( Base64decode( &sealedWorldKey[0], sealedWorldKeyEncoded.c_str() ) );

        String sealingKey( Base64decode_len( sealingKeyEncoded.c_str() ), '\0' );
        sealingKey.resize( Base64decode( &sealingKey[0], sealingKeyEncoded.c_str() ) );

        String plainWorldKey( m_keyUtilities.worldKeySize(), '\0' );
        m_keyUtilities.unsealWorldKey( &sealedWorldKey[0], &sealingKey[0], &plainWorldKey[0] );
        m_joinKey = plainWorldKey;
    }
#endif
}

void Tela::eraseInviteeKey()
{
#ifdef __EMSCRIPTEN__
    {
    LiteStream stream;
    stream << "sessionStorage.removeItem(\"" << _sealedWorldKey << "\");";
    emscripten_run_script_string( stream.str().c_str() );
    }

    {
    LiteStream stream;
    stream << "sessionStorage.removeItem(\"" << _sealingKey << "\");";
    emscripten_run_script_string( stream.str().c_str() );
    }

    m_joinKey.clear();
#endif
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
