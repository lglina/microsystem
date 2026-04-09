#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Authenticator.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "Updater.h"

#include <cstdio>
#include <filesystem>
#include <map>
#include <string>

using namespace std::filesystem;

namespace
{
    const Agape::String defaultUpdateStream( "stable" );
} // Anonymous namespace

namespace Agape
{

namespace Stratus
{

Updater::Updater( TupleRouter& tupleRouter, Authenticator& authenticator ) :
  m_tupleRouter( tupleRouter ),
  m_authenticator( authenticator ),
  m_currentFile( nullptr ),
  m_currentFileSize( 0 ),
  Native( _UpdateServer )
{
    m_tupleRouter.registerActor( this );
}

Updater::~Updater()
{
    m_tupleRouter.deregisterActor( this );

    if( m_currentFile )
    {
        std::fclose( m_currentFile );
    }
}

bool Updater::accept( Tuple& tuple )
{
    bool handled( false );

    String tupleType( TupleRouter::tupleType( tuple ) );
    if( tupleType == _UpdateMetadataRequest )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Received UpdateMetadataRequest" );
#endif

        bool success( false );

        updateFromFilesystem();

        String updateStream( m_authenticator.updateStream() );
        if( updateStream.empty() ) updateStream = defaultUpdateStream;
#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Update stream for client is " + updateStream );
#endif

        success = ( m_streamMetadata.find( updateStream.c_str() ) != m_streamMetadata.end() );
#ifdef LOG_LOADERS
        if( success )
        {
            LiteStream stream;
            stream << "Updater: Found firmware file version "
                   << m_streamMetadata[updateStream.c_str()].m_version
                   << " for stream "
                   << updateStream;
            LOG_DEBUG( stream.str() );
        }
        else
        {
            LOG_DEBUG( "Updater: No firmware file found for update stream " + updateStream );
        }
#endif

        Tuple response;
        TupleRouter::setSourceActor( response, _UpdateServer );
        TupleRouter::setSourceID( response, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( response, _UpdateMetadataResponse );
        response[_version] = success ? m_streamMetadata[updateStream.c_str()].m_version : -1;
        response[_success] = success ? 1 : 0;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Sending UpdateMetadataResponse" );
#endif

        m_tupleRouter.route( response );

        handled = true;
    }
    else if( tupleType == _UpdateOpenRequest )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Received UpdateOpenRequest" );
#endif

        bool success( false );

        updateFromFilesystem();

        String updateStream( m_authenticator.updateStream() );
        if( updateStream.empty() ) updateStream = defaultUpdateStream;
#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Update stream for client is " + updateStream );
#endif

        success = ( m_streamMetadata.find( updateStream.c_str() ) != m_streamMetadata.end() );
        if( success )
        {
#ifdef LOG_LOADERS
            LiteStream stream;
            stream << "Updater: Found firmware file version "
                   << m_streamMetadata[updateStream.c_str()].m_version
                   << " for stream "
                   << updateStream;
            LOG_DEBUG( stream.str() );
#endif

            if( m_currentFile )
            {
                std::fclose( m_currentFile );
                m_currentFile = nullptr;
            }
            m_currentFileSize = 0;

            LiteStream filenameStream;
            filenameStream << updateStream << ".bin";
            m_currentFile = std::fopen( filenameStream.str().c_str(), "rb" );
            if( m_currentFile )
            {
                if( ( std::fseek( m_currentFile, 0, SEEK_END ) == 0 ) &&
                    ( ( m_currentFileSize = std::ftell( m_currentFile ) ) != -1L ) )
                {
                    success = true;
                }
                else
                {
                    LOG_DEBUG( "Updater: Failed to find size of firmware file " + filenameStream.str() );
                }
            }
            else
            {
                LOG_DEBUG( "Updater: Failed to open firmware file " + filenameStream.str() );
            }
        }
        else
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Updater: No firmware file found for update stream " + updateStream );
#endif
        }

        Tuple response;
        TupleRouter::setSourceActor( response, _UpdateServer );
        TupleRouter::setSourceID( response, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( response, _UpdateOpenResponse );
        response[_size] = (double)m_currentFileSize;
        response[_success] = success ? 1 : 0;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Sending UpdateMetadataResponse" );
#endif
        m_tupleRouter.route( response );

        handled = true;
    }
    else if( tupleType == _UpdateReadRequest )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Received UpdateReadRequest" );
#endif

        bool success( false );

        Tuple response;
        TupleRouter::setSourceActor( response, _UpdateServer );
        TupleRouter::setSourceID( response, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( response, _UpdateReadResponse );

        if( m_currentFile )
        {
            int offset( tuple[_offset] );
            int length( tuple[_length] );

            if( ( offset >= 0 ) &&
                ( ( offset + length ) <= m_currentFileSize ) )
            {
                if( std::fseek( m_currentFile, offset, SEEK_SET ) == 0 )
                {
                    String data( length, '\0' );
                    size_t numRead( std::fread( &data[0], 1, length, m_currentFile ) );
                    if( ( numRead > 0 ) &&
                        !std::feof( m_currentFile ) &&
                        !std::ferror( m_currentFile ) )
                    {
                        data.resize( numRead );

                        bool isEmpty( true );
                        for( int i = 0; i < numRead; ++i )
                        {
                            if( data[i] != '\xff' )
                            {
                                isEmpty = false;
                                break;
                            }
                        }

                        response[_offset] = offset;
                        response[_length] = (double)numRead;
                        if( isEmpty )
                        {
                            response[_empty] = 1;
                        }
                        else
                        {
                            response[_data] = data;
                            response[_data].markBinary();
                        }

                        success = true;
                    }
                    else
                    {
                        LOG_DEBUG( "Updater: Short read/EOF/error reading update file" );
                    }
                }
                else
                {
                    LiteStream stream;
                    stream << "Updater: Failed to seek to offset "
                           << offset
                           << ". File size is "
                           << m_currentFileSize;
                    LOG_DEBUG( stream.str() );
                }
            }
            else
            {
                LiteStream stream;
                stream << "Updater: Offset "
                       << offset
                       << " and/or length "
                       << length
                       << " invalid. File size is "
                       << m_currentFileSize;
                LOG_DEBUG( stream.str() );
            }
        }
        else
        {
            LOG_DEBUG( "Updater: Firmware file not open" );
        }

        response[_success] = success ? 1 : 0;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Updater: Sending UpdateReadResponse" );
#endif
        m_tupleRouter.route( response );

        handled = true;
    }

    return handled;
}

void Updater::updateFromFilesystem()
{
    for( auto const& directoryEntry : directory_iterator( "." ) )
    {
#ifdef LOG_LOADERS
        LiteStream dstream;
        dstream << directoryEntry.path().filename().c_str() << "; "
                << directoryEntry.path().extension().c_str() << "; "
                << (directoryEntry.is_regular_file() ? "r" : "n");
        LOG_DEBUG( dstream.str() );
#endif
        if( directoryEntry.is_regular_file() &&
            ( directoryEntry.path().extension() == ".bin" ) )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( String( "Updater: Found firmware file " ) + directoryEntry.path().filename().c_str() );
#endif
            std::string streamName = directoryEntry.path().stem();
            if( m_streamMetadata.find( streamName ) != m_streamMetadata.end() )
            {
                if( m_streamMetadata[streamName].m_lastModified < directoryEntry.last_write_time() )
                {
#ifdef LOG_LOADERS
                    LOG_DEBUG( "Updater: File changed on disk. Re-reading version from file." );
#endif
                    m_streamMetadata[streamName].m_lastModified = directoryEntry.last_write_time();
                    int version( versionFromFile( directoryEntry.path().filename() ) );
                    if( version != -1 )
                    {
#ifdef LOG_LOADERS
                        LiteStream stream;
                        stream << "Updater: New version for "
                               << streamName.c_str()
                               << " is "
                               << version;
                        LOG_DEBUG( stream.str() );
#endif
                        m_streamMetadata[streamName].m_version = version;
                    }
                    else
                    {
#ifdef LOG_LOADERS
                        LOG_DEBUG( String( "Updater: Erasing metadata for stream " ) + streamName.c_str() );
#endif
                        m_streamMetadata.erase( streamName );
                    }
                }
                else
                {
#ifdef LOG_LOADERS
                    LOG_DEBUG( "Updater: File not changed on disk since last read" );
#endif
                }
            }
            else
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "Updater: Reading version from file" );
#endif
                int version( versionFromFile( directoryEntry.path().filename() ) );
                if( version != -1 )
                {
                    struct UpdateMetadata updateMetadata;
                    updateMetadata.m_version = version;
                    updateMetadata.m_lastModified = directoryEntry.last_write_time();
                    m_streamMetadata[streamName] = updateMetadata;
                }
            }
        }
    }
}

int Updater::versionFromFile( const std::string& filename )
{
    std::FILE* file( std::fopen( filename.c_str(), "rb" ) );
    if( file )
    {
        if( std::fseek( file, -8, SEEK_END ) == 0 )
        {
            int version( 0 );
            if( std::fread( (char*)( &version ), 1, sizeof( int ), file ) == sizeof( int ) )
            {
                std::fclose( file );
                return version;
            }
            else
            {
                LOG_DEBUG( String( "Updater: Unable to read version from file " ) + filename.c_str() );
                if( std::feof( file ) ) LOG_DEBUG( "EOF" );
                if( std::ferror( file ) ) LOG_DEBUG( "ERROR" );
            }
        }
        else
        {
            LOG_DEBUG( String( "Updater: Unable to seek in file " ) + filename.c_str() );
        }

        std::fclose( file );
    }
    else
    {
        LOG_DEBUG( String( "Updater: Unable to open file " ) + filename.c_str() );
    }

    return -1;
}

} // namespace Stratus

} // namespace Agape
