#include "Collections.h"
#include "KiamaFSAssetLoader.h"
#include "KiamaFS.h"
#include "String.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"

#include "Loggers/Logger.h"

namespace Agape
{

namespace AssetLoaders
{

KiamaFS::KiamaFS( const World::Coordinates& coordinates,
                  const String& name,
                  const String& extension,
                  Agape::KiamaFS& fs,
                  Map< String, int >& index ) :
  AssetLoader( coordinates, name ),
  m_extension( extension ),
  m_fs( fs ),
  m_index( index ),
  m_file( nullptr ),
  m_currentOffset( 0 )
{
}

KiamaFS::~KiamaFS()
{
    if( m_file )
    {
        delete( m_file );
    }
}

bool KiamaFS::open()
{
    LiteStream filenameStream;
    filenameStream << m_name << "." << m_extension;

    LOG_DEBUG( "Attempting to load " + filenameStream.str() + " with KiamaFS" );

    /*
    if( m_index.find( filenameStream.str() ) != m_index.end() )
    {
        LOG_DEBUG( "Found in filesystem index" );
        m_file = new Agape::KiamaFS::File( m_fs.file( filenameStream.str() ) );
        m_file->open( Agape::KiamaFS::File::OpenMode::readMode );
        return true;
    }
    else
    {
        LOG_DEBUG( "NOT found in filesystem index" );
        return false;
    }
    */

    m_file = m_fs.file( filenameStream.str() );
    return( m_file->open( Agape::KiamaFS::File::OpenMode::readMode ) );
}

int KiamaFS::read( char* data, int offset, int len )
{
    if( offset < m_file->size() )
    {
        int lengthRemaining( m_file->size() - offset );
        int lengthToRead( len < lengthRemaining ? len : lengthRemaining );
        if( offset != m_currentOffset )
        {
            m_file->seek( offset );
        }
        
        int lengthRead( m_file->read( data, lengthToRead ) );
        
        LiteStream debugStream;
        //debugStream << "Read " << lengthRead << " bytes from KiamaFS file";
        LOG_DEBUG( debugStream.str() );

        m_currentOffset += lengthToRead;

        return lengthRead;
    }
    else
    {
        return 0;
    }
}

int KiamaFS::size()
{
    return m_file->size();
}

} // namespace AssetLoaders

} // namespace Agape
