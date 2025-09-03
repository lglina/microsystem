#ifndef AGAPE_VALUE_H
#define AGAPE_VALUE_H

#include "Collections.h"
#include "EncryptableDecryptable.h"
#include "ReadableWritable.h"
#include "String.h"

namespace Agape
{

class Encryptor;
class ReadableWritable;

namespace Linda2
{
class Tuple;
} // namespace Linda2

// We could use std::variant, but it's nice to have backward
// compatibility with older standards, and perhaps we reduce
// memory bloat by writing our own?

class Value;
class ValueLoader;

typedef Vector< Value* >::iterator ListIterator;
typedef Vector< Value* >::const_iterator ConstListIterator;
typedef Map< String, Value* >::iterator MapIterator;
typedef Map< String, Value* >::const_iterator ConstMapIterator;

class Value : public EncryptableDecryptable
{
public:
    enum ValueType
    {
        unknown,
        word,
        number,
        list,
        map,
        binary
    };

    Value();
    Value( const String& stringValue );
    Value( const char* cStringValue );
    Value( int intValue );
    Value( double doubleValue );
    Value( Vector< Value* > listValue );
    Value( Map< String, Value* > mapValue );
    Value( const Value& other );

    virtual ~Value();

    Value* push_back( Value* value );
    ListIterator listBegin();
    ConstListIterator listBegin() const;
    ListIterator listEnd();
    ConstListIterator listEnd() const;

    MapIterator mapBegin();
    ConstMapIterator mapBegin() const;
    MapIterator mapEnd();
    ConstMapIterator mapEnd() const;

    bool hasValue( const String& key ) const;

    ListIterator erase( const ListIterator& it );
    MapIterator erase( const MapIterator& it );
    MapIterator erase( const String& key );
    
    const Value& operator[]( const String& key ) const;
    Value& operator[]( const String& key );
    const Value& operator[]( const char* key ) const;
    Value& operator[]( const char* key );

    String& operator=( const String& stringValue );
    const char* operator=( const char* cStringValue );
    int operator=( int intValue );
    double operator=( double doubleValue );
    Vector< Value* >& operator=( const Vector< Value* > listValue );
    Map< String, Value* >& operator=( const Map< String, Value* > mapValue );
    Value& operator=( const Value& other );

    operator const String&() const;
    operator const char*() const;
    operator int() const;
#ifdef __WATCOMC__
    operator double() const;
#else
    explicit operator double() const; // Prefer cast to int over double.
#endif
    operator const Vector< Value* >&() const;
    operator const Map< String, Value* >&() const;

    bool toReadableWritable( ReadableWritable& rw ) const;
    static bool fromReadableWritable( ReadableWritable& rw, Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    enum ValueType type() const;

    void markBinary();

    bool isNull() const;

    void setValueLoader( ValueLoader* valueLoader );
    bool load();
    bool save();
    void setParent( Value* parent );

    String toString() const;

    int rawSize() const;
    const char* raw() const;

    String dump( int indent = 0 ) const;

private:
    class StringSerialiser : public ReadableWritable
    {
    public:
        StringSerialiser();
        virtual int read( char* data, int len );
        virtual int write( const char* data, int len );
        virtual bool error();

        String m_data;
        int m_offset;
    };

    static char valueTypeToChar( enum ValueType valueType );
    static enum ValueType valueTypeFromChar( char c );

    void reset();

    void deleteString();
    void deleteVector();
    void deleteMap();

    friend bool operator==( const Value& lhs, const Value& rhs );
    friend bool operator!=( const Value& lhs, const Value& rhs );

#ifdef __WATCOMC__
    friend bool operator==( const Value& lhs, const String& rhs );
#endif

    friend Linda2::Tuple; // For Tuple::dump().

    enum ValueType m_valueType;

    String* m_wordValue;
    double m_numberValue;
    Vector< Value* >* m_listValue;
    Map< String, Value* >* m_mapValue;

    Value* m_parent;

    ValueLoader* m_valueLoader;

    static String m_emptyString;
    static Vector< Value* > m_emptyList;
    static Map< String, Value* > m_emptyMap;
    static Value m_emptyValue;
};

bool operator==( const Value& lhs, const Value& rhs );
bool operator!=( const Value& lhs, const Value& rhs );

#ifdef __WATCOMC__
bool operator==( const Value& lhs, const String& rhs ); // Help WATCOM in value-to-string comparison
#endif

bool listsEqual( const Vector< Value* >& lhs, const Vector< Value* >& rhs );
bool mapsEqual( const Map< String, Value* >& lhs, const Map< String, Value* >& rhs );

} // namespace Agape

#endif // AGAPE_VALUE_H
