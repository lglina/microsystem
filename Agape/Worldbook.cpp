#include "World/User.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "ConfigurationStore.h"
#include "String.h"
#include "StringConstants.h"
#include "Worldbook.h"

using namespace Agape::World;

using Agape::String;

namespace Agape
{

Worldbook::Worldbook( ConfigurationStore& configurationStore ) :
  m_configurationStore( configurationStore )
{
}

void Worldbook::add( const Metadata& metadata, bool setDefault )
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    
    Value newWorld;
    metadata.toValue( newWorld, true, false ); // true = serialise secret world key, false = don't serialise users list.
    worlds[metadata.m_worldID] = newWorld;

    if( setDefault )
    {
        setDefaultWorldID( metadata.m_worldID );
    }

    m_configurationStore.save();
}

void Worldbook::remove( const String& worldID )
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    worlds.erase( worldID );

    if( hasDefaultWorldID() && ( getDefaultWorldID() == worldID ) )
    {
        m_configurationStore.remove( _defaultWorldID );
    }

    m_configurationStore.save();
}

Vector< String > Worldbook::getNames() const
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    Vector< String > names;
    ConstMapIterator it( worlds.mapBegin() );
    for( ; it != worlds.mapEnd(); ++it )
    {
        Metadata metadata( Metadata::fromValue( *( it->second ) ) );
        names.push_back( metadata.m_name );
    }

    return names;
}

Vector< String > Worldbook::getIDs() const
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    Vector< String > ids;
    ConstMapIterator it( worlds.mapBegin() );
    for( ; it != worlds.mapEnd(); ++it )
    {
        ids.push_back( it->first );
    }

    return ids;
}

bool Worldbook::getMetadata( const String& worldID, Metadata& metadata ) const
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    if( worlds.hasValue( worldID ) )
    {
        metadata = Metadata::fromValue( worlds[worldID], true, false ); // true = deserialise secret world key, false = don't deserialise users list.
        return true;
    }

    return false;
}

Vector< Metadata > Worldbook::getAllMetadata() const
{
    Value& worldsValues( m_configurationStore.get( _worlds ) );
    Vector< Metadata > worlds;
    ConstMapIterator it( worldsValues.mapBegin() );
    for( ; it != worldsValues.mapEnd(); ++it )
    {
        worlds.push_back( Metadata::fromValue( *( it->second ), true, false ) ); // true = deserialise secret world key, false = don't deserialise users list.
    }

    return worlds;
}

bool Worldbook::getWorldIDByName( const String& name, String& worldID ) const
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    ConstMapIterator it( worlds.mapBegin() );
    for( ; it != worlds.mapEnd(); ++it )
    {
        Metadata metadata( Metadata::fromValue( *( it->second ) ) );
        if( metadata.m_name == name )
        {
            worldID = metadata.m_worldID;
            return true;
        }
    }

    return false;
}

bool Worldbook::getWorldNameByID( const String& worldID, String& name ) const
{
    Value& worlds( m_configurationStore.get( _worlds ) );
    if( worlds.hasValue( worldID ) )
    {
        name = worlds[worldID][_name];
        return true;
    }

    return false;
}

bool Worldbook::hasDefaultWorldID() const
{
    return( m_configurationStore.hasKey( _defaultWorldID ) );
}

String Worldbook::getDefaultWorldID() const
{
    if( m_configurationStore.hasKey( _defaultWorldID ) )
    {
        return m_configurationStore.get( _defaultWorldID );
    }

    return String();
}

void Worldbook::setDefaultWorldID( const String& worldID )
{
    m_configurationStore.get( _defaultWorldID ) = worldID;

    m_configurationStore.save();
}

bool Worldbook::getUserForWorld( const String& worldID, User& user ) const
{
    const Value& users( m_configurationStore.get( _users ) );
    
    if( users.hasValue( worldID ) )
    {
        user = User::fromValue( users[worldID] );
        return true;
    }

    return false;
}

void Worldbook::setUserForWorld( const String& worldID, const User& user )
{
    Value& users( m_configurationStore.get( _users ) );
    
    Value userValue;
    user.toValue( userValue );
    users[worldID] = userValue;

    m_configurationStore.save();
}

void Worldbook::removeUserForWorld( const String& worldID )
{
    Value& users( m_configurationStore.get( _users ) );

    if( users.hasValue( worldID ) )
    {
        users.erase( worldID );
    }

    m_configurationStore.save();
}

} // namespace Agape
