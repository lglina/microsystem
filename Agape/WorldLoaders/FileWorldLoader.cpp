#include "Utils/EscapeBase64.h"
#include "Utils/LiteStream.h"
#include "World/WorldMetadata.h"
#include "FileWorldLoader.h"
#include "FileWriter.h"
#include "String.h"
#include "Value.h"

#include <unistd.h>

namespace Agape
{

namespace WorldLoaders
{

File::File( const String& path,
            const String& extension ) :
  m_path( path ),
  m_extension( extension )
{
}

bool File::create( const World::Metadata& metadata, String& reason )
{
    FileWriter fileWriter( worldFilename( metadata.m_worldID ), FileWriter::modeWrite );

    if( fileWriter.isOpen() )
    {
        Value worldValue;
        metadata.toValue( worldValue );
        worldValue.toReadableWritable( fileWriter );
    }

    return true;
}

bool File::join( World::Metadata& metadata, String& reason )
{
    return create( metadata, reason );
}

bool File::load( World::Metadata& metadata, String& reason )
{
    FileWriter fileWriter( worldFilename( metadata.m_worldID ), FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value worldValue;
        Value::fromReadableWritable( fileWriter, worldValue );
        metadata = World::Metadata::fromValue( worldValue );
        return true;
    }

    return false;
}

String File::worldFilename( const String& worldID )
{
    LiteStream filenameStream;
    filenameStream << m_path << "/"
                   << escapeBase64( worldID ).substr( 0, 8 ) << "."
                   << m_extension;
    return filenameStream.str();
}

} // namespace WorldLoaders

} // namespace Agape
