#ifndef AGAPE_LINDA2_TUPLE_H
#define AGAPE_LINDA2_TUPLE_H

#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

class ReadableWritable;

#ifdef __GNUC__
class MongoDocumentBuilder;
#endif

namespace Linda2
{

class Tuple
{
#ifdef __GNUC__
    friend MongoDocumentBuilder;
#endif

public:
    typedef Map< String, Value >::iterator Iterator;
    typedef Map< String, Value >::const_iterator ConstIterator;

    Tuple();
    Tuple( const Map< String, Value >& values );

    bool hasValue( const String& key ) const;
    const Value& operator[]( const String& key ) const;
    Value& operator[]( const String& key );
    const Value& operator[]( const char* key ) const;
    Value& operator[]( const char* key );

    bool toReadableWritable( ReadableWritable& rw ) const;
    static bool fromReadableWritable( ReadableWritable& rw, Tuple& tuple );

    Iterator begin();
    ConstIterator begin() const;
    Iterator end();
    ConstIterator end() const;

    String dump() const;

private:
    Map< String, Value > m_values;

    static Value m_emptyValue;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_H
