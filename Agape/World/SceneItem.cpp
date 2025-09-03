#include "Clocks/Clock.h"
#include "Encryptors/Encryptor.h"
#include "Utils/base64/base64.h"
#include "Utils/Snowflake.h"
#include "Utils/StrToHex.h"
#include "SceneItem.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

#include <string.h>

namespace
{
    // N.B. These should match the lengths within this file and the header.
    const int _maxAssetNameLength( 31 );
    const int _maxActionLength( 63 );
} // Anonymous namespace

namespace Agape
{

namespace World
{

SceneItem::SceneItem() :
  m_row( 0 ),
  m_col( 0 ),
  m_height( 0 ),
  m_width( 0 ),
  m_action( nullptr ),
  m_flags( none )
{
    ::memset( m_assetName, 0, 48 );
    ::memset( m_dateTime, 0, 14 );
    ::memset( m_owner, 0, 16 );
    
    String snowflake;
    snowflake = Snowflake::generate();
    ::memcpy( m_snowflake, &snowflake[0], 16 );

    snowflake = Snowflake::generate();
    ::memcpy( m_modificationSnowflake, &snowflake[0], 16 );
}

SceneItem::SceneItem( const String& assetName,
                      int row,
                      int col,
                      int height,
                      int width,
                      const String& owner,
                      const String& action,
                      Clock& clock ) :
  m_row( row ),
  m_col( col ),
  m_height( height ),
  m_width( width ),
  m_action( nullptr ),
  m_flags( none )
{
    ::memset( m_assetName, 0, 48 );
    setAssetName( assetName );

    clock.fillDateTime( m_dateTime );

    ::memset( m_owner, 0, 16 );
    if( owner.length() == 16 )
    {
        ::memcpy( m_owner, &owner[0], 16 );
    }

    setAction( action );

    String snowflake;
    snowflake = Snowflake::generate();
    ::memcpy( m_snowflake, &snowflake[0], 16 );

    snowflake = Snowflake::generate();
    ::memcpy( m_modificationSnowflake, &snowflake[0], 16 );
}

SceneItem::SceneItem( const SceneItem& other ) :
  m_row( 0 ),
  m_col( 0 ),
  m_height( 0 ),
  m_width( 0 ),
  m_action( nullptr ),
  m_flags( none )
{
    *this = other;
}

SceneItem::~SceneItem()
{
    if( m_action ) delete[]( m_action );
}

String SceneItem::assetName() const
{
    if( !( m_flags & encrypted ) )
    {
        // Plaintext asset name is null-terminated string,
        // max 32B (including terminator).
        return String( m_assetName );
    }

    return String();
}

int SceneItem::row() const
{
    return m_row;
}

int SceneItem::col() const
{
    return m_col;
}

int SceneItem::height() const
{
    return m_height;
}

int SceneItem::width() const
{
    return m_width;
}

String SceneItem::dateTime() const
{
    return String( m_dateTime, 14 );
}

String SceneItem::owner() const
{
    return String( m_owner, 16 );
}

String SceneItem::action() const
{
    if( m_action && !( m_flags & encrypted ) )
    {
        // Plaintext action is null-terminated string,
        // max 64B (including terminator).
        return String( m_action );
    }

    return String();
}

String SceneItem::snowflake() const
{
    return String( m_snowflake, 16 );
}

String SceneItem::modificationSnowflake() const
{
    return String( m_modificationSnowflake, 16 );
}

char SceneItem::flags() const
{
    return m_flags;
}

void SceneItem::setAssetName( const String& assetName )
{
    if( ( assetName.length() < 32 ) && !( m_flags & encrypted ) )
    {
        ::strncpy( m_assetName, assetName.c_str(), 31 );
        m_assetName[31] = '\0';
    }
}

void SceneItem::setRow( int row )
{
    m_row = row;
}

void SceneItem::setCol( int col )
{
    m_col = col;
}

void SceneItem::setDimensions( int height, int width )
{
    m_height = height;
    m_width = width;
}

void SceneItem::setAction( const String& action )
{
    if( !action.empty() &&
        ( action.length() < 64 ) &&
        !( m_flags & encrypted ) )
    {
        if( !m_action ) m_action = new char[80];
        ::memset( m_action, 0, 80 );
        ::strncpy( m_action, action.c_str(), 63 );
        m_action[63] = '\0';
    }
    else if( !( m_flags & encrypted ) ) // && ( empty || too long )
    {
        delete[]( m_action );
        m_action = nullptr;
    }
}

void SceneItem::setSnowflake( const String& snowflake )
{
    if( snowflake.length() == 16 )
    {
        ::memcpy( m_snowflake, &snowflake[0], 16 );
    }
}

void SceneItem::setFlags( char flags )
{
    // FIXME: Do we have a mechanism to allow set/clear of individual
    // flags, or do we expect the caller to read/set/write?
    m_flags = flags;
}

void SceneItem::touch()
{
    String snowflake = Snowflake::generate();
    ::memcpy( m_modificationSnowflake, &snowflake[0], 16 );
}

int SceneItem::maxAssetNameLength()
{
    return _maxAssetNameLength;
}

int SceneItem::maxActionLength()
{
    return _maxActionLength;
}

void SceneItem::toValue( Value& value ) const
{
    if( m_flags & encrypted )
    {
        // Encode 48B fixed-length encrypted name to Base-64.
        String encodedName( Base64encode_len( 48 ), '\0' );
        Base64encode( &encodedName[0],
                      m_assetName,
                      48 );
        encodedName.resize( encodedName.length() - 1 );
        value[_assetName] = encodedName;
    }
    else
    {
        // Null-terminated string.
        value[_assetName] = m_assetName;
    }

    value[_row] = m_row;
    value[_column] = m_col;
    value[_height] = m_height;
    value[_width] = m_width;
    value[_dateTime] = String( m_dateTime, 14 );
    value[_owner] = String( m_owner, 16 );

    if( m_action )
    {
        if( m_flags & encrypted )
        {
            // Encode 80B fixed-length encrypted action to Base-64.
            String encodedAction( Base64encode_len( 80 ), '\0' );
            Base64encode( &encodedAction[0],
                          m_action,
                          80 );
            encodedAction.resize( encodedAction.length() - 1 );
            value[_action] = encodedAction;
        }
        else
        {
            // Null-terminated string.
            value[_action] = m_action;
        }
    }

    value[_snowflake] = String( m_snowflake, 16 );
    value[_modificationSnowflake] = String( m_modificationSnowflake, 16 );
    value[_flags] = (int)m_flags;
}

SceneItem SceneItem::fromValue( const Value& value )
{
    SceneItem sceneItem;
    sceneItem.m_flags = (int)value[_flags];

    if( sceneItem.m_flags & encrypted )
    {
        // Decode Base-64 to 48B fixed-length encrypted name.
        const String& encodedName = value[_assetName];
        String decodedName( Base64decode_len( encodedName.c_str() ), '\0' );
        decodedName.resize( Base64decode( &decodedName[0], encodedName.c_str() ) );
        if( decodedName.length() == 48 ) ::memcpy( sceneItem.m_assetName, &decodedName[0], 48 );
    }
    else
    {
        // Create null-terminated string.
        ::strncpy( sceneItem.m_assetName, value[_assetName], 31 );
        sceneItem.m_assetName[31] = '\0';
    }

    sceneItem.m_row = value[_row];
    sceneItem.m_col = value[_column];
    sceneItem.m_height = value[_height];
    sceneItem.m_width = value[_width];
    if( value[_dateTime].rawSize() == 14 ) ::memcpy( sceneItem.m_dateTime, value[_dateTime].raw(), 14 ); // FIXME: Arrgghhhhh!
    if( value[_owner].rawSize() == 16 ) ::memcpy( sceneItem.m_owner, value[_owner].raw(), 16 );
    
    if( value.hasValue(_action) )
    {
        const String& action = value[_action];
        if( !action.empty() )
        {
            if( sceneItem.m_flags & encrypted )
            {
                // Decode Base-64 to 80B fixed-length encrypted action.
                String decodedAction( Base64decode_len( action.c_str() ), '\0' );
                decodedAction.resize( Base64decode( &decodedAction[0], action.c_str() ) );
                if( decodedAction.length() == 80 )
                {
                    sceneItem.m_action = new char[80];
                    ::memcpy( sceneItem.m_action, &decodedAction[0], 80 );
                }
            }
            else
            {
                // Create null-terminated string.
                sceneItem.m_action = new char[80];
                ::memset( sceneItem.m_action, 0, 80 );
                ::strncpy( sceneItem.m_action, value[_action], 63 );
                sceneItem.m_action[63] = '\0';
            }
        }
    }
    
    if( value[_snowflake].rawSize() == 16 ) ::memcpy( sceneItem.m_snowflake, value[_snowflake].raw(), 16 );
    if( value[_modificationSnowflake].rawSize() == 16 ) ::memcpy( sceneItem.m_modificationSnowflake, value[_modificationSnowflake].raw(), 16 );

    return sceneItem;
}

bool SceneItem::encrypt( Encryptor& encryptor )
{
    bool success( true );
    
    // Encrypt all 32B of fixed-length plaintext name buffer to 48B ciphertext
    // and store back into name buffer as binary.
    String encryptedName = encryptor.encrypt( String( m_assetName, 32 ), false ); // false = don't encode to Base-64.
    if( encryptedName.length() == 48 )
    {
        ::memcpy( m_assetName, &encryptedName[0], 48 );
    }
    else
    {
        success = false;
    }

    if( success && m_action )
    {
        // Encrypt all 64B of fixed-length plaintext action buffer to 80B
        // ciphertext and store back into action buffer as binary.
        String encryptedAction = encryptor.encrypt( String( m_action, 64 ), false ); // false = don't encode to Base-64.
        if( encryptedAction.length() == 80 )
        {
            ::memcpy( m_action, &encryptedAction[0], 80 );
        }
        else
        {
            success = false;
        }
    }

    if( success )
    {
        m_flags |= encrypted;
    }

    return success;
}

bool SceneItem::decrypt( Encryptor& encryptor )
{
    bool success( true );

    // Decrypt all 48B of fixed-length ciphertext name buffer to 32B plaintext
    // and store back into name buffer as null-terminated string (plus any
    // extraneous bytes, if any).
    String decryptedName = encryptor.decrypt( String( m_assetName, 48 ), false ); // false = don't decode input from Base-64.
    if( decryptedName.length() == 32 )
    {
        ::memcpy( m_assetName, &decryptedName[0], 32 );
    }
    else
    {
        success = false;
    }

    if( success && m_action )
    {
        // Decrypt all 80B of fixed-length ciphertext action buffer to 64B
        // plaintext and store back into action buffer as null-terminated string
        // (plus any extraneous bytes, if any).
        String decryptedAction = encryptor.decrypt( String( m_action, 80 ), false ); // false = don't decode input from Base-64.
        if( decryptedAction.length() == 64 )
        {
            ::memcpy( m_action, &decryptedAction[0], 64 );
        }
        else
        {
            success = false;
        }
    }

    if( success )
    {
        m_flags &= ~encrypted;
    }

    return success;
}

SceneItem& SceneItem::operator=( const SceneItem& other )
{
    ::memcpy( m_assetName, other.m_assetName, 48 );
    m_row = other.m_row;
    m_col = other.m_col;
    m_height = other.m_height;
    m_width = other.m_width;
    ::memcpy( m_dateTime, other.m_dateTime, 14 );
    ::memcpy( m_owner, other.m_owner, 16 );

    if( other.m_action )
    {
        if( !m_action ) m_action = new char[80];
        ::memcpy( m_action, other.m_action, 80 );
    }
    else
    {
        if( m_action ) delete[]( m_action );
        m_action = nullptr;
    }

    ::memcpy( m_snowflake, other.m_snowflake, 16 );
    ::memcpy( m_modificationSnowflake, other.m_modificationSnowflake, 16 );
    m_flags = other.m_flags;

    return *this;
}

bool SceneItem::operator==( const SceneItem& other ) const
{
    return( ::memcmp( m_snowflake, other.m_snowflake, 16 ) == 0 );
}

bool SceneItem::operator!=( const SceneItem& other ) const
{
    return( !operator==(other ) );
}

bool SceneItem::operator<( const SceneItem& other ) const
{
    return( hexToUll( String( m_modificationSnowflake, 16 ) ) <
            hexToUll( String( other.m_modificationSnowflake, 16 ) ) );
}

} // namespace World

} // namespace Agape
