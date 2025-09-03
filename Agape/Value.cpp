#include "Encryptors/Encryptor.h"
#include "Loggers/Logger.h"
#include "Utils/StrToHex.h"
#include "Utils/LiteStream.h"
#include "ValueLoaders/ValueLoader.h"
#include "Collections.h"
#include "ReadableWritable.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

#include <string.h>

namespace
{
    // Set some basic limits to sanity check sizes when tuples are being
    // deserialised, in case a connection breaks and the link layer doesn't
    // prevent us seeing corrupt/partial data.
    const int maxElements( 256 );
    const int maxStringLength( 1024 );
} // Anonymous namespace

namespace Agape
{

String Value::m_emptyString;
Vector< Value* > Value::m_emptyList;
Map< String, Value* > Value::m_emptyMap;
Value Value::m_emptyValue;

Value::Value() :
  m_valueType( unknown ),
  m_wordValue( nullptr ),
  m_numberValue( 0.0 ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( const String& stringValue ) :
  m_valueType( word ),
  m_wordValue( new String( stringValue ) ),
  m_numberValue( 0.0 ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( const char* cStringValue ) :
  m_valueType( word ),
  m_wordValue( new String( cStringValue ) ),
  m_numberValue( 0.0 ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( int intValue ) :
  m_valueType( number ),
  m_wordValue( nullptr ),
  m_numberValue( intValue ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( double doubleValue ) :
  m_valueType( number ),
  m_wordValue( nullptr ),
  m_numberValue( doubleValue ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( Vector< Value* > listValue ) :
  m_valueType( list ),
  m_wordValue( nullptr ),
  m_numberValue( 0.0 ),
  m_listValue( new Vector< Value* >( listValue ) ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( Map< String, Value* > mapValue ) :
  m_valueType( map ),
  m_wordValue( nullptr ),
  m_numberValue( 0.0 ),
  m_listValue( nullptr ),
  m_mapValue( new Map< String, Value* >( mapValue ) ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
}

Value::Value( const Value& other ) :
  m_valueType( unknown ),
  m_wordValue( nullptr ),
  m_numberValue( 0.0 ),
  m_listValue( nullptr ),
  m_mapValue( nullptr ),
  m_parent( nullptr ),
  m_valueLoader( nullptr )
{
    *this = other;
}

Value::~Value()
{
    reset();

    if( m_valueLoader )
    {
        delete( m_valueLoader );
    }
}

Value* Value::push_back( Value* value )
{
    if( m_valueType == unknown )
    {
        m_valueType = list;
        m_listValue = new Vector< Value* >;
    }

    if( m_valueType == list )
    {
        m_listValue->push_back( value );
    }
    
    return value;
}

ListIterator Value::listBegin()
{
    if( m_valueType == list )
    {
        return m_listValue->begin();
    }
    else
    {
        return m_emptyList.begin();
    }
}

ConstListIterator Value::listBegin() const
{
    if( m_valueType == list )
    {
        return m_listValue->begin();
    }
    else
    {
        return m_emptyList.begin();
    }
}

ListIterator Value::listEnd()
{
    if( m_valueType == list )
    {
        return m_listValue->end();
    }
    else
    {
        return m_emptyList.end();
    }
}

ConstListIterator Value::listEnd() const
{
    if( m_valueType == list )
    {
        return m_listValue->end();
    }
    else
    {
        return m_emptyList.end();
    }
}

MapIterator Value::mapBegin()
{
    if( m_valueType == map )
    {
        return m_mapValue->begin();
    }
    else
    {
        return m_emptyMap.begin();
    }
}

ConstMapIterator Value::mapBegin() const
{
    if( m_valueType == map )
    {
        return m_mapValue->begin();
    }
    else
    {
        return m_emptyMap.begin();
    }
}

MapIterator Value::mapEnd()
{
    if( m_valueType == map )
    {
        return m_mapValue->end();
    }
    else
    {
        return m_emptyMap.end();
    }
}

ConstMapIterator Value::mapEnd() const
{
    if( m_valueType == map )
    {
        return m_mapValue->end();
    }
    else
    {
        return m_emptyMap.end();
    }
}

bool Value::hasValue( const String& key ) const
{
    if( m_valueType == list )
    {
        Vector< Value* >::const_iterator it( m_listValue->begin() );
        for( ; it != m_listValue->end(); ++it )
        {
            if( **it == key )
            {
                return true;
            }
        }

        return false;
    }
    else if( m_valueType == map )
    {
        Map< String, Value* >::const_iterator it( m_mapValue->find( key ) );
        return( it != m_mapValue->end() );
    }

    return false;
}

ListIterator Value::erase( const ListIterator& it )
{
    if( m_valueType == list )
    {
        return m_listValue->erase( it );
    }
    else
    {
        return m_emptyList.end();
    }
}

MapIterator Value::erase( const MapIterator& it )
{
    if( m_valueType == map )
    {
#ifndef __WATCOMC__
        return m_mapValue->erase( it );
#else
        m_mapValue->erase( it );
        return m_mapValue->begin();
#endif
    }
    else
    {
        return m_emptyMap.end();
    }
}

MapIterator Value::erase( const String& key )
{
    if( m_valueType == map )
    {
        MapIterator it( m_mapValue->find( key ) );
#ifndef __WATCOMC__
        return m_mapValue->erase( it );
#else
        m_mapValue.erase( it );
        return m_mapValue.begin();
#endif
    }

    return m_emptyMap.end();
}

const Value& Value::operator[]( const String& key ) const
{
    if( m_valueType == map )
    {
        Map< String, Value* >::const_iterator it( m_mapValue->find( key ) );
        if( it != m_mapValue->end() )
        {
            return *( it->second );
        }
    }

    return m_emptyValue;
}

Value& Value::operator[]( const String& key )
{
    if( m_valueType == map )
    {
        Map< String, Value* >::iterator it( m_mapValue->find( key ) );
        if( it != m_mapValue->end() )
        {
            return *( it->second );
        }
        else
        {
            ( *m_mapValue )[key] = new Value;
            return *( ( *m_mapValue )[key] );
        }
    }
    else if( m_valueType == unknown )
    {
        m_valueType = map;
        m_mapValue = new Map< String, Value* >;
        ( *m_mapValue )[key] = new Value;
        return *( ( *m_mapValue )[key] );
    }

    return m_emptyValue;
}

const Value& Value::operator[]( const char* key ) const
{
    return operator[]( String( key ) );
}

Value& Value::operator[]( const char* key )
{
    return operator[]( String( key ) );
}

String& Value::operator=( const String& stringValue )
{
    reset();
    m_valueType = word;
    m_wordValue = new String( stringValue );
    return *m_wordValue;
}

const char* Value::operator=( const char* cStringValue )
{
    reset();
    m_valueType = word;
    m_wordValue = new String( cStringValue );
    return m_wordValue->c_str();
}

int Value::operator=( int intValue )
{
    reset();
    m_valueType = number;
    m_numberValue = intValue;
    return m_numberValue;
}

double Value::operator=( double doubleValue )
{
    reset();
    m_valueType = number;
    m_numberValue = doubleValue;
    return m_numberValue;
}

Vector< Value* >& Value::operator=( const Vector< Value* > listValue )
{
    reset();
    m_valueType = list;
    m_listValue = new Vector< Value* >( listValue );
    return *m_listValue;
}

Map< String, Value* >& Value::operator=( const Map< String, Value* > mapValue )
{
    reset();
    m_valueType = map;
    m_mapValue = new Map< String, Value* >( mapValue );
    return *m_mapValue;
}

Value& Value::operator=( const Value& other )
{
    reset();

    Vector< Value* >::const_iterator listIt;
    Map< String, Value* >::const_iterator mapIt;

    switch( other.m_valueType )
    {
    case word:
        m_valueType = word;
        m_wordValue = new String( *( other.m_wordValue ) );
        break;
    case number:
        m_valueType = number;
        m_numberValue = other.m_numberValue;
        break;
    case list:
        m_valueType = list;
        m_listValue = new Vector< Value* >;
        // Deep copy
        for( listIt = other.m_listValue->begin(); listIt != other.m_listValue->end(); ++listIt )
        {
            m_listValue->push_back( new Value( **listIt ) );
        }
        break;
    case map:
        m_valueType = map;
        m_mapValue = new Map< String, Value* >;
        // Deep copy
        for( mapIt = other.m_mapValue->begin(); mapIt != other.m_mapValue->end(); ++mapIt )
        {
            ( *m_mapValue )[mapIt->first] = new Value( *( mapIt->second ) );
        }
        break;
    case binary:
        m_valueType = binary;
        m_wordValue = new String( *( other.m_wordValue ) );
        break;
    default: // Also captures other.m_valueType == unknown.
        break;
    }

    // Note: ValueLoader NOT copied from other.

    return *this;
}

Value::operator const String&() const
{
    if( ( m_valueType == word ) || ( m_valueType == binary ) )
    {
        return *m_wordValue;
    }

    return m_emptyString;
}

Value::operator const char*() const
{
    if( m_valueType == word )
    {
        return m_wordValue->c_str();
    }

    return m_emptyString.c_str();
}

Value::operator int() const
{
    return m_numberValue;
}

Value::operator double() const
{
    return m_numberValue;
}

Value::operator const Vector< Value* >&() const
{
    if( m_valueType == list )
    {
        return *m_listValue;
    }

    return m_emptyList;
}

Value::operator const Map< String, Value* >&() const
{
    if( m_valueType == map )
    {
        return *m_mapValue;
    }

    return m_emptyMap;
}

bool Value::toReadableWritable( ReadableWritable& rw ) const
{
    bool success( true );

    char type( valueTypeToChar( m_valueType ) );
    
    // Write value type.
    success = ( rw.write( &type, 1 ) == 1 );

    if( success )
    {
        switch( m_valueType )
        {
        case word:
        case binary:
            {
                // Write word length, then word.
                int strLen( m_wordValue->length() );
                success = ( rw.write( (char*)&strLen, 4 ) == 4 );
                if( success ) success = ( rw.write( m_wordValue->c_str(), strLen ) == strLen ); // Doesn't write NUL from c_str().
            }
            break;
        case number:
            // Write number.
            success = ( rw.write( (char*)&m_numberValue, sizeof( m_numberValue ) ) == sizeof( m_numberValue ) );
            break;
        case list:
            {
                // Write list size.
                int listSize( m_listValue->size() );
                success = ( rw.write( (char*)&listSize, 4 ) == 4 );
                
                Vector< Value* >::const_iterator listIt( m_listValue->begin() );
                for( ; success && ( listIt != m_listValue->end() ); ++listIt )
                {
                    // Recurse to write list item.
                    success = ( *listIt )->toReadableWritable( rw );
                }
            }
            break;
        case map:
            {
                // Write map size.
                int mapSize( m_mapValue->size() );
                success = ( rw.write( (char*)&mapSize, 4 ) == 4 );
                
                Map< String, Value* >::const_iterator mapIt( m_mapValue->begin() );
                for( ; success && ( mapIt != m_mapValue->end() ); ++mapIt )
                {
                    // Write key length, then key.
                    int keyStrLen( mapIt->first.length() );
                    success = ( rw.write( (char*)&keyStrLen, 4 ) == 4 );
                    if( success ) success = ( rw.write( mapIt->first.c_str(), keyStrLen ) == keyStrLen );

                    // Recurse to write value.
                    if( success ) success = mapIt->second->toReadableWritable( rw );
                }
            }
            break;
        default:
            break;
        }
    }

    return success;
}

bool Value::fromReadableWritable( ReadableWritable& rw, Value& value )
{
    bool success( true );

    // Read value type.
    char type;
    success = ( rw.read( &type, 1, ReadableWritable::rwBlock ) == 1 );

    if( success )
    {
        value.reset();
        value.m_valueType = valueTypeFromChar( type );

        switch( value.m_valueType )
        {
        case word:
        case binary:
            {
                // Read word length.
                int strLen;
                success = ( rw.read( (char*)&strLen, 4, ReadableWritable::rwBlock ) == 4 );

                if( success &&
                    ( ( strLen < 0 ) || ( strLen > maxStringLength ) ) )
                {
                    LiteStream stream;
                    stream << "Value: Word/binary length negative or too long: " << strLen;
                    LOG_DEBUG( stream.str() );
                    success = false;
                }

                if( success )
                {
                    // Read word.
                    char* str( new char[strLen] );
                    success = ( rw.read( str, strLen, ReadableWritable::rwBlock ) == strLen );
                    if( success ) value.m_wordValue = new String( str, strLen );
                    delete[]( str );
                }
            }
            break;
        case number:
            // Read number.
            success = ( rw.read( (char*)&value.m_numberValue, sizeof( value.m_numberValue ), ReadableWritable::rwBlock ) == sizeof( value.m_numberValue ) );
            break;
        case list:
            value.m_listValue = new Vector< Value* >;
            // Read list length.
            int listLen;
            success = ( rw.read( (char*)&listLen, 4, ReadableWritable::rwBlock ) == 4 );

            if( success &&
                ( ( listLen < 0 ) || ( listLen > maxElements ) ) )
            {
                LiteStream stream;
                stream << "Tuple: List length negative or too long: " << listLen;
                LOG_DEBUG( stream.str() );
                success = false;
            }

            for( int i = 0; success && ( i < listLen ); ++i )
            {
                // Recurse to read list items.
                Value* listItem = new Value;
                success = Value::fromReadableWritable( rw, *listItem );
                if( success ) value.m_listValue->push_back( listItem );
            }
            break;
        case map:
            value.m_mapValue = new Map< String, Value* >;
            // Read map length.
            int mapLen;
            success = ( rw.read( (char*)&mapLen, 4, ReadableWritable::rwBlock ) == 4 );

            if( success &&
                ( ( mapLen < 0 ) || ( mapLen > maxElements ) ) )
            {
                LiteStream stream;
                stream << "Tuple: Map length negative or too long: " << mapLen;
                LOG_DEBUG( stream.str() );
                success = false;
            }

            for( int i = 0; success && ( i < mapLen ); ++i )
            {
                // Read key length.
                int firstLen( 0 );
                success = ( rw.read( (char*)&firstLen, 4, ReadableWritable::rwBlock ) == 4 );

                if( success &&
                    ( ( firstLen < 0 ) || ( firstLen > maxStringLength ) ) )
                {
                    LiteStream stream;
                    stream << "Value: Map value key length negative or too long: " << firstLen;
                    LOG_DEBUG( stream.str() );
                    success = false;
                }

                if( success )
                {
                    // Read key.
                    char* first( new char[firstLen + 1] );
                    success = ( rw.read( first, firstLen, ReadableWritable::rwBlock ) == firstLen );
                    first[firstLen] = '\0';

                    if( success )
                    {
                        // Recurse to read value.
                        Value* second = new Value();
                        success = Value::fromReadableWritable( rw, *second );
                        if( success ) ( *( value.m_mapValue ) )[first] = second;
                    }

                    delete[]( first );
                }
            }
            break;
        default:
            break;
        }
    }

    return success;
}

bool Value::encrypt( Encryptor& encryptor )
{
    StringSerialiser serialiser;
    if( toReadableWritable( serialiser ) )
    {
        *this = encryptor.encrypt( serialiser.m_data );
        return true;
    }

    return false;
}

bool Value::decrypt( Encryptor& encryptor )
{
    if( isNull() ) return true;
    StringSerialiser serialiser;
    serialiser.m_data = encryptor.decrypt( *this );
    return( fromReadableWritable( serialiser, *this ) );
}

enum Value::ValueType Value::type() const
{
    return m_valueType;
}

void Value::markBinary()
{
    m_valueType = binary;
}

bool Value::isNull() const
{
    return( m_valueType == unknown );
}

void Value::setValueLoader( ValueLoader* valueLoader )
{
    m_valueLoader = valueLoader;
}

bool Value::load()
{
    if( m_valueLoader ) { return m_valueLoader->load( *this ); }
    return true; // no-op
}

bool Value::save()
{
    if( m_valueLoader ) { return m_valueLoader->save( *this ); }
    if( m_parent && m_parent->m_valueLoader ) { return m_parent->m_valueLoader->save( *m_parent ); }
    return true; // no-op
}

void Value::setParent( Value* parent )
{
    m_parent = parent;
}

String Value::toString() const
{
    LiteStream stream;
    if( m_valueType == word )
    {
        stream << *m_wordValue;
    }
    else if( m_valueType == number )
    {
        if( (long long)m_numberValue == m_numberValue  )
        {
            stream << (long long)m_numberValue;
        }
        else
        {
            stream << m_numberValue;
        }
    }

    return stream.str();
}

int Value::rawSize() const
{
    if( ( m_valueType == word ) || ( m_valueType == binary ) )
    {
        return m_wordValue->size();
    }

    return 0;
}

const char* Value::raw() const
{
    if( ( m_valueType == word ) || ( m_valueType == binary ) )
    {
        return &( ( *m_wordValue )[0] );
    }

    return NULL;
}

String Value::dump( int indent ) const
{
    String s;
    LiteStream stream;

#ifndef NDEBUG
    switch( m_valueType )
    {
    case word:
        s += *m_wordValue + "\r\n";
        break;
    case number:
        stream << m_numberValue;
        s += stream.str() + "\r\n";
        break;
    case list:
        {
        Vector< Value* >::const_iterator listIt( m_listValue->begin() );

        for( int i = 0; i < indent - 4; ++i )
        {
            s += ' ';
        }
        
        s += "[\r\n";

        for( ; listIt != m_listValue->end(); ++listIt )
        {
            s += ( *listIt )->dump( indent + 4 );
        }

        for( int i = 0; i < indent - 4; ++i )
        {
            s += ' ';
        }

        s += "]\r\n";
        }
        break;
    case map:
        {
        Map< String, Value* >::const_iterator mapIt( m_mapValue->begin() );

        for( int i = 0; i < indent - 4; ++i )
        {
            s += ' ';
        }

        s += "{\r\n";

        for( ; mapIt != m_mapValue->end(); ++mapIt )
        {
            for( int i = 0; i < indent; ++i )
            {
                s += ' ';
            }

            s += mapIt->first + ":";
            if( ( mapIt->second->m_valueType == list ) ||
                ( mapIt->second->m_valueType == map ) )
            {
                s += "\r\n";
            }
            else
            {
                s += ' ';
            }

            bool printable( true );
            if( mapIt->second->m_valueType == word )
            {
                for( int i = 0; i < mapIt->second->m_wordValue->length(); ++i )
                {
                    if( ( *( mapIt->second->m_wordValue ) )[i] < 0x20 )
                    {
                        printable = false;
                        break;
                    }
                }
            }
            else if( mapIt->second->m_valueType == binary )
            {
                printable = false;
            }

            if( printable )
            {
                s += mapIt->second->dump( indent + 4 );
            }
            else
            {
                s += "\r\n";
                s += toHexStr( mapIt->second->m_wordValue->c_str(), mapIt->second->m_wordValue->length() );
            }
        }

        for( int i = 0; i < indent - 4; ++i )
        {
            s += ' ';
        }

        s += "}\r\n";
        }
        break;
    case binary:
        s += toHexStr( m_wordValue->c_str(), m_wordValue->length() ) + "\r\n";
        break;
    default:
        s += "((UNKNOWN))\r\n";
        break;
    }
#endif

    return s;
}

Value::StringSerialiser::StringSerialiser() :
  m_offset( 0 )
{
}

int Value::StringSerialiser::read( char* data, int len )
{
    if( ( m_offset + len ) <= m_data.length() )
    {
        ::memcpy( data, &m_data[m_offset], len );
        m_offset += len;
        return len;
    }

    return 0;
}

int Value::StringSerialiser::write( const char* data, int len )
{
    if( ( m_offset + len ) > m_data.length() ) m_data.resize( m_offset + len );
    ::memcpy( &m_data[m_offset], data, len );
    m_offset += len;
    return len;
}

bool Value::StringSerialiser::error()
{
    return false;
}

char Value::valueTypeToChar( enum ValueType valueType )
{
    switch( valueType )
    {
    case word:
        return 0;
    case number:
        return 1;
    case list:
        return 2;
    case map:
        return 3;
    case binary:
        return 4;
    case unknown:
    default:
        return -1;
    }
}

enum Value::ValueType Value::valueTypeFromChar( char c )
{
    switch( c )
    {
    case 0:
        return word;
    case 1:
        return number;
    case 2:
        return list;
    case 3:
        return map;
    case 4:
        return binary;
    case -1:
    default:
        return unknown;
    }
}

void Value::reset()
{
    switch( m_valueType )
    {
    case word:
    case binary:
        deleteString();
        break;
    case number:
        m_numberValue = 0.0;
        break;
    case list:
        deleteVector();
        break;
    case map:
        deleteMap();
        break;
    default:
        break;
    }
    m_valueType = unknown;
}

void Value::deleteString()
{
    delete( m_wordValue );
    m_wordValue = nullptr;
}

void Value::deleteVector()
{
    Vector< Value* >::const_iterator it( m_listValue->begin() );
    for( ; it != m_listValue->end(); ++it )
    {
        delete( *it );
    }

    delete( m_listValue );
    m_listValue = nullptr;
}

void Value::deleteMap()
{
    Map< String, Value* >::const_iterator it( m_mapValue->begin() );
    for( ; it != m_mapValue->end(); ++it )
    {
        delete( it->second );
    }

    delete( m_mapValue );
    m_mapValue = nullptr;
}

bool operator==( const Value& lhs, const Value& rhs )
{
    if( ( ( lhs.m_valueType == Value::word ) || ( lhs.m_valueType == Value::binary ) ) &&
        ( ( rhs.m_valueType == Value::word ) || ( rhs.m_valueType == Value::binary ) ) )
    {
        return( *( lhs.m_wordValue ) == *( rhs.m_wordValue ) );
    }
    else if( ( lhs.m_valueType == Value::number ) && ( rhs.m_valueType == Value::number ) )
    {
        return( lhs.m_numberValue == rhs.m_numberValue );
    }
    else if( ( lhs.m_valueType == Value::list ) && ( rhs.m_valueType == Value::list ) )
    {
        return listsEqual( *( lhs.m_listValue ), *( rhs.m_listValue ) );
    }
    else if( ( lhs.m_valueType == Value::map ) && ( rhs.m_valueType == Value::map ) )
    {
        return mapsEqual( *( lhs.m_mapValue ), *( rhs.m_mapValue ) );
    }
    else if( ( lhs.m_valueType == Value::unknown ) && ( rhs.m_valueType == Value::unknown ) )
    {
        return true;
    }

    return false;
}

bool operator!=( const Value& lhs, const Value& rhs )
{
    return( !( lhs == rhs ) );
}

#ifdef __WATCOMC__
bool operator==( const Value& lhs, const String& rhs )
{
    return( ( lhs.m_valueType == Value::word ) && ( *( lhs.m_wordValue ) == rhs ) );
}
#endif

bool listsEqual( const Vector< Value* >& lhs, const Vector< Value* >& rhs )
{
    // FIXME: Lists could have different numbers of repeated values?
    // FIXME: On^2 complexity!!
    Vector< Value* >::const_iterator lhsIt( lhs.begin() );
    for( ; lhsIt != lhs.end(); ++lhsIt )
    {
        Vector< Value* >::const_iterator rhsIt( rhs.begin() );
        for( ; rhsIt != rhs.end(); ++rhsIt )
        {
            if( **rhsIt == **lhsIt )
            {
                break;
            }
        }

        if( rhsIt == rhs.end() )
        {
            return false;
        }
    }

    return true;
}

bool mapsEqual( const Map< String, Value* >& lhs, const Map< String, Value* >& rhs )
{
    // FIXME: rhs could have spurious values?
    Map< String, Value* >::const_iterator lhsIt( lhs.begin() );
    for( ; lhsIt != lhs.end(); ++lhsIt )
    {
        Map< String, Value* >::const_iterator rhsIt( rhs.find( lhsIt->first ) );
        if( ( rhsIt == rhs.end() ) || ( *( rhsIt->second ) != *( lhsIt->second ) ) )
        {
            return false;
        }
    }

    return true;
}

} // namespace Agape
