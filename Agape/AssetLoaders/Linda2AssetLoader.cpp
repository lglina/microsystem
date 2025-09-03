#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "AssetLoader.h"
#include "Linda2AssetLoader.h"
#include "Promise.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include <string.h>

namespace
{
    const int maxBlockSize( 256 );
} // Anonymous namespace

namespace Agape
{

namespace AssetLoaders
{

Linda2::Linda2( const World::Coordinates& coordinates,
                const String& name,
                TupleRouter& tupleRouter,
                Timers::Factory& timerFactory,
                const String& collectionName ) :
  AssetLoader( coordinates, name ),
  Native( "AssetLoader" ),
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_isLoading( false ),
  m_collectionName( collectionName ),
  m_isOpen( false ),
  m_openMode( modeRead ),
  m_size( 0 ),
  m_readData( nullptr ),
  m_readDataLen( 0 )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2AssetLoader: Created" );
#endif
    m_tupleRouter.registerActor( this );

    m_readData = new char[maxBlockSize];
}

Linda2::~Linda2()
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2AssetLoader: Destructing" );
#endif
    close(); // Ensure closed, so remote responder can close its asset loader.
    m_tupleRouter.deregisterActor( this );
    delete[]( m_readData );
}

bool Linda2::open()
{
    return open( modeRead, String() );
}

bool Linda2::open( enum OpenMode openMode, const String& linkedItem )
{
    bool success( true );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _AssetLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _AssetOpenRequest );
    tuple[_assetName] = m_name;
    tuple[_collectionName] = m_collectionName;
    tuple[_openMode] = ( openMode == modeWrite ) ? _write : _read;
    tuple[_linkedItem] = linkedItem;
    m_coordinates.toValue( tuple[_coordinates] );

    m_openMode = openMode;

    m_isLoading = true;

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2AssetLoader: Sending AssetOpenRequest" );
#endif
    m_assetOpenResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_assetOpenResponse.getFuture().get();

    m_isLoading = false;

    return success;
}

int Linda2::read( char* data, int offset, int len )
{
    if( m_isOpen && ( m_openMode == modeRead ) )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "Read " << m_name << " offset " << offset << " len " << len;
        LOG_DEBUG( stream.str() );
#endif

        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _AssetLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _AssetReadRequest );
        tuple[_assetName] = m_name;
        tuple[_collectionName] = m_collectionName;
        tuple[_offset] = offset;
        int lenToRead( ( len > maxBlockSize ) ? maxBlockSize : len );
        tuple[_length] = lenToRead;
        m_coordinates.toValue( tuple[_coordinates] );

        m_isLoading = true;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoader: Sending AssetReadRequest" );
#endif
        m_assetReadResponse = Promise( &m_tupleRouter, &m_timerFactory );
        if( m_tupleRouter.route( tuple ) )
        {
            if( m_assetReadResponse.getFuture().get() && ( m_readDataLen == lenToRead ) )
            {
                ::memcpy( data, m_readData, m_readDataLen );
                m_isLoading = false;
                return m_readDataLen;
            }
        }

        m_isLoading = false;
    }

    return 0;
}

int Linda2::write( const char* data, int offset, int len )
{
    if( m_isOpen && ( m_openMode == modeWrite ) )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "Write " << m_name << " offset " << offset << " len " << len;
        LOG_DEBUG( stream.str() );
#endif

        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _AssetLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _AssetWriteRequest );
        tuple[_assetName] = m_name;
        tuple[_collectionName] = m_collectionName;
        tuple[_offset] = offset;
        int lenToWrite( ( len > maxBlockSize ) ? maxBlockSize : len );
        tuple[_length] = lenToWrite;
        String dataStr( len, '\0' );
        ::memcpy( &dataStr[0], data, lenToWrite );
        tuple[_data] = dataStr;
        tuple[_data].markBinary();
        m_coordinates.toValue( tuple[_coordinates] );

        m_isLoading = true;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoader: Sending AssetWriteRequest" );
#endif
        m_assetWriteResponse = Promise( &m_tupleRouter, &m_timerFactory );
        if( m_tupleRouter.route( tuple ) )
        {
            Value lenWritten;
            if( m_assetWriteResponse.getFuture().get( lenWritten ) )
            {
                m_isLoading = false;
                return lenWritten;
            }
        }

        m_isLoading = false;
    }

    return 0;
}

bool Linda2::close()
{
    if( m_isOpen )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _AssetLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _AssetCloseRequest );
        tuple[_assetName] = m_name;
        tuple[_collectionName] = m_collectionName;
        m_coordinates.toValue( tuple[_coordinates] );

        m_isLoading = true;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoader: Sending AssetCloseRequest" );
#endif
        m_assetCloseResponse = Promise( &m_tupleRouter, &m_timerFactory );
        if( m_tupleRouter.route( tuple ) )
        {
            if( m_assetCloseResponse.getFuture().get() )
            {
                m_isOpen = false;
                m_isLoading = false;
                return true;
            }
        }

        m_isLoading = false;
    }

    return false;
}

int Linda2::size()
{
    // Received during open.
    return m_size;
}

bool Linda2::move( const String& newName )
{
    if( !m_isOpen )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _AssetLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _AssetMoveRequest );
        tuple[_assetName] = m_name;
        tuple[_collectionName] = m_collectionName;
        tuple[_newName] = newName;
        m_coordinates.toValue( tuple[_coordinates] );

        m_isLoading = true;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoader: Sending AssetMoveRequest" );
#endif
        m_assetMoveResponse = Promise( &m_tupleRouter, &m_timerFactory );
        if( m_tupleRouter.route( tuple ) )
        {
            if( m_assetMoveResponse.getFuture().get() )
            {
                m_isLoading = false;
                return true;
            }
        }

        m_isLoading = false;
    }

    return false;
}

bool Linda2::erase()
{
    if( !m_isOpen )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _AssetLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _AssetEraseRequest );
        tuple[_assetName] = m_name;
        tuple[_collectionName] = m_collectionName;
        m_coordinates.toValue( tuple[_coordinates] );

        m_isLoading = true;

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoader: Sending AssetEraseRequest" );
#endif
        m_assetEraseResponse = Promise( &m_tupleRouter, &m_timerFactory );
        if( m_tupleRouter.route( tuple ) )
        {
            if( m_assetEraseResponse.getFuture().get() )
            {
                m_isLoading = false;
                return true;
            }
        }

        m_isLoading = false;
    }

    return false;
}

bool Linda2::error()
{
    return m_tupleRouter.routeError();
}

bool Linda2::accept( Tuple& tuple )
{
    bool handled( false );

    if( ( TupleRouter::tupleType( tuple ) == _AssetOpenResponse ) &&
        ( tuple[_collectionName] == m_collectionName ) &&
        m_isLoading )
    {
        m_assetOpenResponse.set( tuple[_success] );
        m_isOpen = ( (int)tuple[_success] == 1 );
        m_size = tuple[_size];

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetReadResponse ) &&
             ( tuple[_collectionName] == m_collectionName ) &&
             m_isLoading )
    {
        if( m_assetReadResponse.set( tuple[_success] ) )
        {
            m_readDataLen = tuple[_length];
            if( ( m_readDataLen == tuple[_data].rawSize() ) && ( m_readDataLen <= maxBlockSize ) )
            {
                ::memcpy( m_readData, tuple[_data].raw(), m_readDataLen );
            }
            else
            {
                m_readDataLen = 0; // Should never happen.
            }
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetWriteResponse ) &&
             ( tuple[_collectionName] == m_collectionName ) &&
             m_isLoading )
    {
        m_assetWriteResponse.set( tuple[_success], tuple[_length] );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetCloseResponse ) &&
             ( tuple[_collectionName] == m_collectionName ) &&
             m_isLoading )
    {
        m_assetCloseResponse.set( tuple[_success] );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetMoveResponse ) &&
             ( tuple[_collectionName] == m_collectionName ) &&
             m_isLoading )
    {
        m_assetMoveResponse.set( tuple[_success] );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetEraseResponse ) &&
             ( tuple[_collectionName] == m_collectionName ) &&
             m_isLoading )
    {
        m_assetEraseResponse.set( tuple[_success] );
        handled = true;
    }

    return handled;
}

} // namespace AssetLoaders

} // namespace Agape
