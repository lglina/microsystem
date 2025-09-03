#ifndef AGAPE_MONGO_DOCUMENT_BUILDER_H
#define AGAPE_MONGO_DOCUMENT_BUILDER_H

#include <String.h>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>

namespace Agape
{

namespace Linda2
{
//class Tuple;
} // namespace Linda2

class Value;

namespace Databases
{

namespace MongoDB
{

class DocumentBuilder
{
public:
    //static bsoncxx::document::value build( const Linda2::Tuple& tuple );
    static bsoncxx::document::value build( const Value& value );

    //static Linda2::Tuple unbuild( bsoncxx::document::view document );
    static Value unbuild( bsoncxx::document::view document );

private:
    static void appendLinda2Value( bsoncxx::builder::basic::sub_document& builder, const String& key, const Value& value );
    static void appendLinda2Value( bsoncxx::builder::basic::sub_array& array, const Value& value );

    static Value getLinda2Value( const bsoncxx::document::element& element );
    static Value getLinda2Value( const bsoncxx::array::element element );
};

} // namespace MongoDB

} // namespace Databases

} // namespace Agape

#endif // AGAPE_MONGO_DOCUMENT_BUILDER_H
