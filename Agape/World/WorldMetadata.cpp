#include "Encryptors/Encryptor.h"
#include "Collections.h"
#include "String.h"
#include "StringConstants.h"
#include "User.h"
#include "Value.h"
#include "WorldMetadata.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace World
{

Metadata::Metadata() :
  m_writable( true )
{
}

void Metadata::toValue( Value& value, bool keys, bool users ) const
{
    value[_name] = m_name;
    value[_worldID] = m_worldID;
    value[_worldAuthKey] = m_worldAuthKey;
    if( keys ) value[_worldKey] = m_worldKey;
    value[_itemKey] = m_itemKey;
    value[_privateKey] = m_privateKey;

    value[_writable] = m_writable ? 1 : 0;

    if( users )
    {
        Value& usersValue( value[_users] );
        Vector< User >::const_iterator it( m_users.begin() );
        for( ; it != m_users.end(); ++it )
        {
            Value* user( new Value );
            it->toValue( *user );
            usersValue.push_back( user );
        }
    }
}

Metadata Metadata::fromValue( const Value& value, bool keys, bool users )
{
    Metadata metadata;

    metadata.m_name = value[_name];
    metadata.m_worldID = value[_worldID];
    metadata.m_worldAuthKey = value[_worldAuthKey];
    if( keys ) metadata.m_worldKey = value[_worldKey];
    metadata.m_itemKey = value[_itemKey];
    metadata.m_privateKey = value[_privateKey];

    metadata.m_writable = ( (int)value[_writable] == 1 );

    if( users )
    {
        const Value& usersValue( value[_users] );
        ConstListIterator it( usersValue.listBegin() );
        for( ; it != usersValue.listEnd(); ++it )
        {
            User user( User::fromValue( **it ) );
            metadata.m_users.push_back( user );
        }
    }

    return metadata;
}

bool Metadata::encrypt( Encryptor& encryptor )
{
    m_name = encryptor.encrypt( m_name );
    m_itemKey = encryptor.encrypt( m_itemKey );

    Vector< User >::iterator it( m_users.begin() );
    for( ; it != m_users.end(); ++it )
    {
        it->encrypt( encryptor );
    }

    return true;
}

bool Metadata::decrypt( Encryptor& encryptor )
{
    m_name = encryptor.decrypt( m_name );
    m_itemKey = encryptor.decrypt( m_itemKey );

    Vector< struct User >::iterator it( m_users.begin() );
    for( ; it != m_users.end(); ++it )
    {
        it->decrypt( encryptor );
    }

    return true;
}

} // namespace World

} // namespace Agape
