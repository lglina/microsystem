#include "Loggers/Logger.h"
#include "Collections.h"
#include "MongoDocumentBuilder.h"
#include "String.h"
#include "Value.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types/bson_value/value.hpp>

#include <stdint.h>
#include <string>

namespace Agape
{

namespace Databases
{

namespace MongoDB
{

bsoncxx::document::value DocumentBuilder::build( const Value& value )
{
    bsoncxx::builder::basic::document builder;
    if( value.type() == Value::map )
    {
        ConstMapIterator it( value.mapBegin() );
        for( ; it != value.mapEnd(); ++it )
        {
            appendLinda2Value( builder, it->first, *( it->second ) );
        }
    }

    return builder.extract();
}

Value DocumentBuilder::unbuild( bsoncxx::document::view document )
{
    Value value;

    bsoncxx::document::view::const_iterator it( document.begin() );
    for( ; it != document.end(); ++it )
    {
        if( it->type() != bsoncxx::type::k_oid ) // Skip OIDs.
        {
            std::string key( it->key() );
            value[ (Agape::StringBase)key ] = getLinda2Value( *it );
        }
    }

    return value;
}

void DocumentBuilder::appendLinda2Value( bsoncxx::builder::basic::sub_document& builder, const String& key, const Value& value )
{
    if( value.type() == Value::word )
    {
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key, (std::string)value ) );
    }
    else if( value.type() == Value::number )
    {
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key, (double)value) );
    }
    else if( value.type() == Value::list )
    {
        bsoncxx::builder::basic::array array;
        ConstListIterator it( value.listBegin() );
        for( ; it != value.listEnd(); ++it )
        {
            appendLinda2Value( array, **it );
        }
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key, array ) );
    }
    else if( value.type() == Value::map )
    {
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key,
            [=]( bsoncxx::builder::basic::sub_document builder ) {
                ConstMapIterator it( value.mapBegin() );
                for( ; it != value.mapEnd(); ++it )
                {
                    appendLinda2Value( builder, it->first, *( it->second ) );
                }
            }
        ) );
    }
    else if( value.type() == Value::binary )
    {
        bsoncxx::types::b_binary binaryData { bsoncxx::binary_sub_type::k_binary,
                                              value.rawSize(),
                                              (const uint8_t*)value.raw() };
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key, binaryData ) );
    }
    else if( value.type() == Value::unknown )
    {
        bsoncxx::types::b_null nullValue;
        builder.append( bsoncxx::builder::basic::kvp( (std::string)key, nullValue ) );
    }
}

void DocumentBuilder::appendLinda2Value( bsoncxx::builder::basic::sub_array& array, const Value& value )
{
    if( value.type() == Value::word )
    {
        array.append( (std::string)value );
    }
    else if( value.type() == Value::number )
    {
        array.append( (double)value );
    }
    else if( value.type() == Value::list )
    {
        bsoncxx::builder::basic::array array;
        ConstListIterator it( value.listBegin() );
        for( ; it != value.listEnd(); ++it )
        {
            appendLinda2Value( array, **it );
        }
    }
    else if( value.type() == Value::map )
    {
        array.append(
            [=]( bsoncxx::builder::basic::sub_document document ) {
                ConstMapIterator it( value.mapBegin() );
                for( ; it != value.mapEnd(); ++it )
                {
                    appendLinda2Value( document, it->first, *( it->second ) );
                }
            }
        );
    }
    else if( value.type() == Value::binary )
    {
        bsoncxx::types::b_binary binaryData { bsoncxx::binary_sub_type::k_binary,
                                              value.rawSize(),
                                              (const uint8_t*)value.raw() };
        array.append( binaryData );
    }
    else if( value.type() == Value::unknown )
    {
        bsoncxx::types::b_null nullValue;
        array.append( nullValue );
    }
}

Value DocumentBuilder::getLinda2Value( const bsoncxx::document::element& element )
{
    Value value;

    // FIXME: Catch exceptions!!
    if( element.type() == bsoncxx::type::k_string )
    {
        std::string str( (bsoncxx::stdx::string_view)( element.get_string() ) );
        // FIXME: Yuck! Agape::String needs a new constructor to allow this to convert directly...
        value = Agape::String( &str[0], str.length() );
    }
    else if( element.type() == bsoncxx::type::k_double )
    {
        value = element.get_double();
    }
    else if( element.type() == bsoncxx::type::k_int32 )
    {
        value = element.get_int32();
    }
    else if( ( element.type() == bsoncxx::type::k_binary ) &&
             ( element.get_binary().sub_type == bsoncxx::binary_sub_type::k_binary ) )
    {
        value = Agape::String( (char*)element.get_binary().bytes, element.get_binary().size );
        value.markBinary();
    }
    else if( element.type() == bsoncxx::type::k_array )
    {
        bsoncxx::array::view arrayView( element.get_array() );
        bsoncxx::array::view::const_iterator it( arrayView.begin() );
        for( ; it != arrayView.end(); ++it )
        {
            value.push_back( new Value( getLinda2Value( *it ) ) );
        }
    }
    else if( element.type() == bsoncxx::type::k_document )
    {
        bsoncxx::document::view document( element.get_document().view() );
        bsoncxx::document::view::const_iterator it( document.begin() );
        for( ; it != document.end(); ++it )
        {
            std::string key( it->key() );
            value[ (Agape::StringBase)key ] = getLinda2Value( *it );
        }
    }
    else if( element.type() == bsoncxx::type::k_null )
    {
        LOG_DEBUG( "MongoDocumentBuilder: Null BSON type!" );
    }
    else
    {
        LOG_DEBUG( "MongoDocumentBuilder: Unknown BSON type!" );
    }

    return value;
}

Value DocumentBuilder::getLinda2Value( const bsoncxx::array::element element )
{
    // FIXME: Copypasta from above, but bsoncxx::array::element can't be
    // treated as bsoncxx::document::element.
    Value value;

    // FIXME: Catch exceptions!!
    if( element.type() == bsoncxx::type::k_string )
    {
        std::string str( (bsoncxx::stdx::string_view)( element.get_string() ) );
        // FIXME: Yuck! Agape::String needs a new constructor to allow this to convert directly...
        value = Agape::String( &str[0], str.length() );
    }
    else if( element.type() == bsoncxx::type::k_double )
    {
        value = element.get_double();
    }
    else if( ( element.type() == bsoncxx::type::k_binary ) &&
             ( element.get_binary().sub_type == bsoncxx::binary_sub_type::k_binary ) )
    {
        value = Agape::String( (char*)element.get_binary().bytes, element.get_binary().size );
        value.markBinary();
    }
    else if( element.type() == bsoncxx::type::k_array )
    {
        bsoncxx::array::view arrayView( element.get_array() );
        bsoncxx::array::view::const_iterator it( arrayView.begin() );
        for( ; it != arrayView.end(); ++it )
        {
            value.push_back( new Value( getLinda2Value( *it ) ) );
        }
    }
    else if( element.type() == bsoncxx::type::k_document )
    {
        bsoncxx::document::view document( element.get_document().view() );
        bsoncxx::document::view::const_iterator it( document.begin() );
        for( ; it != document.end(); ++it )
        {
            std::string key( it->key() );
            value[ (Agape::StringBase)key ] = getLinda2Value( *it );
        }
    }
    else if( element.type() == bsoncxx::type::k_null )
    {
        LOG_DEBUG( "MongoDocumentBuilder: Null BSON type!" );
    }
    else
    {
        LOG_DEBUG( "MongoDocumentBuilder: Unknown BSON type!" );
    }

    return value;
}

} // namespace MongoDB

} // namespace Databases

} // namespace Agape
