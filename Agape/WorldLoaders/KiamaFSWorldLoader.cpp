#include "Utils/EscapeBase64.h"
#include "Utils/LiteStream.h"
#include "World/WorldMetadata.h"
#include "KiamaFSWorldLoader.h"
#include "KiamaFS.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace WorldLoaders
{

KiamaFS::KiamaFS( Agape::KiamaFS& fs ) :
  m_fs( fs )
{
}

bool KiamaFS::create( const World::Metadata& metadata, String& reason )
{
    LiteStream filenameStream;
    filenameStream << escapeBase64( metadata.m_worldID ).substr( 0, 8 ) << ".wld";

    Agape::KiamaFS::File* file( m_fs.file( filenameStream.str() ) );
    if( file->open( Agape::KiamaFS::File::OpenMode::writeMode ) )
    {
        Value worldValue;
        metadata.toValue( worldValue );
        worldValue.toReadableWritable( *file );
        file->commit();
    }

    return true;
}

bool KiamaFS::join( World::Metadata& metadata, String& reason )
{
    return create( metadata, reason );
}

bool KiamaFS::load( World::Metadata& metadata, String& reason )
{
    LiteStream filenameStream;
    filenameStream << escapeBase64( metadata.m_worldID ).substr( 0, 8 ) << ".wld";

    Agape::KiamaFS::File* file( m_fs.file( filenameStream.str() ) );
    if( file->open( Agape::KiamaFS::File::OpenMode::readMode ) )
    {
        Value worldValue( Value::fromReadableWritable( *file ) );
        metadata = World::Metadata::fromValue( worldValue );
        return true;
    }

    return false;
}

} // namespace WorldLoaders

} // namespace Agape
