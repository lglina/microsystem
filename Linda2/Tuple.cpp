#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "Collections.h"
#include "ReadableWritable.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

namespace
{
    // Set some basic limits to sanity check sizes when tuples are being
    // deserialised, in case a connection breaks and the link layer doesn't
    // prevent us seeing corrupt/partial data.
    const int maxElements( 256 );
    const int maxStringLength( 1024 );
} // Anonymous namespace

using Agape::String;

namespace Agape
{

namespace Linda2
{

Value Tuple::m_emptyValue;

Tuple::Tuple()
{
}

Tuple::Tuple( const Map< String, Value >& values ) :
  m_values( values )
{
}

bool Tuple::hasValue( const String& key ) const
{
    Map< String, Value >::const_iterator it( m_values.find( key ) );
    return( it != m_values.end() );
}

const Value& Tuple::operator[]( const String& key ) const
{
    Map< String, Value >::const_iterator it( m_values.find( key ) );
    if( it != m_values.end() )
    {
        return it->second;
    }
    else
    {
        return m_emptyValue;
    }
}

Value& Tuple::operator[]( const String& key )
{
    Map< String, Value >::iterator it( m_values.find( key ) );
    if( it != m_values.end() )
    {
        return it->second;
    }
    else
    {
        return m_values[key];
    }
}

const Value& Tuple::operator[]( const char* key ) const
{
    return operator[]( String( key ) );
}

Value& Tuple::operator[]( const char* key )
{
    return operator[]( String( key ) );
}

bool Tuple::toReadableWritable( ReadableWritable& rw ) const
{
    bool success( true );

    // Write num. values.
    char numValues( m_values.size() );
    success = ( rw.write( &numValues, 1 ) == 1 );

    Map< String, Value >::const_iterator it( m_values.begin() );
    for( ; success && ( it != m_values.end() ); ++it )
    {
        // Write key length, then key.
        char strLen( it->first.length() );
        success = ( rw.write( &strLen, 1 ) == 1 );
        if( success ) success = ( rw.write( it->first.c_str(), strLen ) == strLen );

        // Write value.
        if( success ) success = it->second.toReadableWritable( rw );
    }

    return success;
}

bool Tuple::fromReadableWritable( ReadableWritable& rw, Tuple& tuple )
{
    bool success( true );

    // Read num. values.
    // Initial read is non-blocking, as then if there's not a tuple waiting
    // we can return to the caller immediately.
    char numValues;
    success = ( rw.read( &numValues, 1, ReadableWritable::rwNonBlock ) == 1 );

    if( success &&
        ( ( numValues < 0 ) || ( numValues > maxElements ) ) )
    {
        LiteStream stream;
        stream << "Tuple: Value count negative or too large: " << numValues;
        LOG_DEBUG( stream.str() );
        success = false;
    }

    for( int i = 0; success && ( i < numValues ); ++i )
    {
        // Read key length.
        char strLen;
        success = ( rw.read( &strLen, 1, ReadableWritable::rwBlock ) == 1 );

        if( success &&
            ( ( strLen < 0 ) || ( strLen > maxStringLength ) ) )
        {
            LiteStream stream;
            stream << "Tuple: Key length negative or too long: " << strLen;
            LOG_DEBUG( stream.str() );
            success = false;
        }
        
        if( success )
        {
            char* str( new char[strLen + 1] );
            success = ( rw.read( str, strLen, ReadableWritable::rwBlock ) == strLen );
            str[strLen] = '\0';

            if( success )
            {
                Value value;
                success = Value::fromReadableWritable( rw, value );
                if( success ) tuple.m_values[str] = value;
            }

            delete[]( str );
        }
    }

    return success;
}

Tuple::Iterator Tuple::begin()
{
    return m_values.begin();
}

Tuple::ConstIterator Tuple::begin() const
{
    return m_values.begin();
}

Tuple::Iterator Tuple::end()
{
    return m_values.end();
}

Tuple::ConstIterator Tuple::end() const
{
    return m_values.end();
}

String Tuple::dump() const
{
    String s;
    
    String type = this->operator[]( String( _type ) );
    if( type != "AssetLoadRequest" &&
        type != "AssetLoadResponse" )
    {
        s += "{\r\n";

#ifndef NDEBUG
        Map< String, Value >::const_iterator it( m_values.begin() );
        for( ; it != m_values.end(); ++it )
        {
            if( ( it->first != _data ) && ( it->first != _sealingKey ) )
            {
                s += "    " + it->first + ":";
                if( ( it->second.m_valueType == Value::list ) ||
                    ( it->second.m_valueType == Value::map ) )
                {
                    s += "\r\n";
                }
                else
                {
                    s += ' ';
                }
                
                s += it->second.dump( 8 );
            }
            else if( it->first == _data )
            {
                String data = it->second;
                s += "    " + it->first + ": " + toHexStr( &data[0], data.length() );
            }
            else if( it->first == _sealingKey )
            {
                s += "    " + it->first + ": ***";
            }
        }
#endif

        s += "}";

        return s;
    }

    return String();
}

} // namespace Linda2

} // namespace Agape
