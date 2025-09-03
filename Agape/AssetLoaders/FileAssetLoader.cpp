#include "Loggers/Logger.h"
#include "Utils/Tokeniser.h"
#include "World/WorldCoordinates.h"
#include "FileAssetLoader.h"
#include "String.h"
#include "Value.h"

#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>

namespace Agape
{

namespace AssetLoaders
{

File::File( const World::Coordinates& coordinates, const String& name, const String& assetPath, const String& extension ) :
  AssetLoader( coordinates, name ),
  m_assetPath( assetPath ),
  m_extension( extension ),
  m_fstream( nullptr ),
  m_size( 0 )
{
}

File::~File()
{
    if( m_fstream )
    {
        delete( m_fstream );
    }
}

bool File::open()
{
    return( open( AssetLoader::modeRead, String() ) );
}

bool File::open( enum OpenMode openMode, const String& linkedItem )
{
    std::ostringstream filenameStrm;
    filenameStrm << m_assetPath << "/" << m_name << "." << m_extension;
#ifdef LOG_LOADERS
    LOG_DEBUG( "Opening asset file:" );
    LOG_DEBUG( filenameStrm.str().c_str() );
#endif

    if( openMode == AssetLoader::modeRead )
    {
        struct stat statBuf;
        int rc = stat( filenameStrm.str().c_str(), &statBuf );
        
        if( rc == 0 && S_ISREG( statBuf.st_mode ) )
        {
            m_size = statBuf.st_size;

            m_fstream = new std::fstream( filenameStrm.str(), std::ios::in | std::ios::binary );
            if( m_fstream->is_open() )
            {
                return true;
            }
        }

        return false;
    }
    else if( openMode == AssetLoader::modeWrite )
    {
        if( !std::filesystem::exists( m_assetPath.c_str() ) )
        {
            std::filesystem::create_directories( m_assetPath.c_str() );
        }

        m_size = 0;
        m_fstream = new std::fstream( filenameStrm.str(), std::ios::out | std::ios::trunc );
        if( m_fstream->is_open() )
        {
            return true;
        }

        return false;
    }

    return false;
}

int File::read( char* data, int offset, int len )
{
    if( m_fstream && ( offset < m_size ) )
    {
        m_fstream->clear(); // In case previous read set eofbit.
        m_fstream->seekg( offset );
        int lengthRemaining( m_size - offset );
        int lengthToRead( len <= lengthRemaining ? len : lengthRemaining );

        char* dataPtr( data );
        int lengthRead( 0 );
        char c;
        m_fstream->get( c );
        while( !m_fstream->eof() && lengthRead < lengthToRead )
        {
            *dataPtr++ = c;
            ++lengthRead;
            m_fstream->get( c );
        }

        return lengthRead;
    }
    else
    {
        return 0;
    }
}

int File::write( const char* data, int offset, int len )
{
    if( m_fstream && ( offset == m_size ) )
    {
        m_fstream->write( data, len );
        m_size += len;
        return len;
    }

    return 0;
}

bool File::close()
{
    if( m_fstream && m_fstream->is_open() )
    {
        m_fstream->close();
    }

    return true;
}

int File::size()
{
    return m_size;
}

bool File::move( const String& newName )
{
    std::ostringstream oldFilenameStrm;
    oldFilenameStrm << m_assetPath << "/" << m_name << "." << m_extension;
    std::string oldFilename( oldFilenameStrm.str() );

    std::ostringstream newFilenameStrm;
    newFilenameStrm << m_assetPath << "/" << newName << "." << m_extension;
    std::string newFilename( newFilenameStrm.str() );

    std::remove( newFilename.c_str() );
    std::rename( oldFilename.c_str(), newFilename.c_str() );

    return true;
}

bool File::erase()
{
    close(); // Ensure fstream closed.

    std::ostringstream filenameStrm;
    filenameStrm << m_assetPath << "/" << m_name << "." << m_extension;
    std::string filename( filenameStrm.str() );

    std::remove( filename.c_str() );

    return true;
}

} // namespace AssetLoaders

} // namespace Agape
