#include "Actors/NativeActors/NativeActor.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "Linda2AssetLoaderResponder.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include <string.h>

using namespace Agape::World;

namespace Agape
{

namespace AssetLoaders
{

Linda2Responder::Linda2Responder( TupleRouter& tupleRouter,
                                  AssetLoaders::Factory& assetLoaderFactory,
                                  const String& collectionName ) :
  Actors::Native( _AssetLoaderResponder ),
  m_tupleRouter( tupleRouter ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_collectionName( collectionName )
{
    // FIXME: Move this to NativeActor.
    m_tupleRouter.registerActor( this );
}

Linda2Responder::~Linda2Responder()
{
    m_tupleRouter.deregisterActor( this );
    
    Map< String, AssetLoader* >::iterator it( m_assetLoaders.begin() );
    for( ; it != m_assetLoaders.end(); ++it )
    {
        delete( it->second );
    }
}

bool Linda2Responder::accept( Tuple& tuple )
{
    bool handled( false );

    if( ( TupleRouter::tupleType( tuple ) == _AssetOpenRequest ) &&
        ( tuple[_collectionName] == m_collectionName ) )
    {
        open( tuple );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetReadRequest ) &&
             ( tuple[_collectionName] == m_collectionName ) )
    {
        read( tuple );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetWriteRequest ) &&
             ( tuple[_collectionName] == m_collectionName ) )
    {
        write( tuple );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetCloseRequest ) &&
             ( tuple[_collectionName] == m_collectionName ) )
    {
        close( tuple );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetMoveRequest ) &&
             ( tuple[_collectionName] == m_collectionName ) )
    {
        move( tuple );
        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _AssetEraseRequest ) &&
             ( tuple[_collectionName] == m_collectionName ) )
    {
        erase( tuple );
        handled = true;
    }

    return handled;
}

void Linda2Responder::reset()
{
}

AssetLoader* Linda2Responder::createAssetLoader( const World::Coordinates& coordinates, const String& assetName )
{
    Map< String, AssetLoader* >::const_iterator it( m_assetLoaders.find( assetName ) );
    if( it != m_assetLoaders.end() )
    {
        return it->second;
    }

    return( m_assetLoaders[assetName] = m_assetLoaderFactory.makeLoader( coordinates, assetName ) );
}

AssetLoader* Linda2Responder::getAssetLoader( const String& assetName )
{
    Map< String, AssetLoader* >::const_iterator it( m_assetLoaders.find( assetName ) );
    if( it != m_assetLoaders.end() )
    {
        return it->second;
    }

    return nullptr;
}

void Linda2Responder::deleteAssetLoader( const String& assetName )
{
    Map< String, AssetLoader* >::const_iterator it( m_assetLoaders.find( assetName ) );
    if( it != m_assetLoaders.end() )
    {
        delete( it->second );
        m_assetLoaders.erase( it );
    }
}

void Linda2Responder::open( const Tuple& tuple )
{
    bool success( true );

    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetOpenResponse );
    response[ _collectionName ] = m_collectionName;

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    String assetName = tuple[_assetName];
    response[_assetName] = assetName;

    String linkedItem = tuple[_linkedItem];
    
    AssetLoader* assetLoader( nullptr );

    if( getAssetLoader( assetName ) )
    {
        LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Received open request while already open" );
        success = false;
    }
    else
    {
        assetLoader = createAssetLoader( coordinates, assetName );
    }

    if( success )
    {
        enum AssetLoader::OpenMode openMode( ( tuple[_openMode] == String( _write ) ) ? AssetLoader::modeWrite : AssetLoader::modeRead );

#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2AssetLoaderResponder: Received open request for " + assetName );
#endif
        
        if( assetLoader->open( openMode, linkedItem ) )
        {
#ifdef LOG_LOADERS
            LiteStream stream;
            stream << "Linda2AssetLoaderResponder: Opened with size " << assetLoader->size();
            LOG_DEBUG( stream.str() );
#endif
            
            response[_size] = assetLoader->size();
        }
        else
        {
            LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Failed to open asset" );
            success = false;
            deleteAssetLoader( assetName );
        }
    }

    response[_success] = success ? 1 : 0;

    m_tupleRouter.route( response );
}

void Linda2Responder::read( const Tuple& tuple )
{
    bool success( true );

    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetReadResponse );
    response[ _collectionName ] = m_collectionName;

    String assetName = tuple[_assetName];
    int offset( tuple[_offset] );
    int length( tuple[_length] );

    AssetLoader* assetLoader( getAssetLoader( assetName ) );
    if( !assetLoader )
    {
        LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Received read request but asset not opened" );
        success = false;
    }

    if( success )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "Linda2AssetLoaderResponder: Received read request for "
               << assetName << " Offset " << offset << " Length " << length;
        LOG_DEBUG( stream.str() );
#endif

        if( offset < 0 )
        {
            LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Offset invalid" );
            success = false;
        }
    }

    if( success )
    {
        String data( length, '\0' );
        int lenRead( assetLoader->read( &data[0], offset, length ) );
        data.resize( lenRead );

        response[_assetName] = assetName;
        response[_offset] = offset;
        response[_length] = lenRead;
        response[_data] = data;
        response[_data].markBinary();
    }

    response[_success] = success ? 1 : 0;

    m_tupleRouter.route( response );
}

void Linda2Responder::write( const Tuple& tuple )
{
    bool success( true );

    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetWriteResponse );
    response[ _collectionName ] = m_collectionName;

    String assetName = tuple[_assetName];
    int offset( tuple[_offset] );
    int length( tuple[_length] );

    AssetLoader* assetLoader( getAssetLoader( assetName ) );
    if( !assetLoader )
    {
        LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Received write request but asset not opened" );
        success = false;
    }

    if( success )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "Linda2AssetLoaderResponder: Received write request for "
               << assetName << " Offset " << offset << " Length " << length;
        LOG_DEBUG( stream.str() );
#endif

        if( ( offset < 0 ) || ( length != tuple[_data].rawSize() ) )
        {
            LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Offset or length invalid" );
            success = false;
        }
    }

    if( success )
    {
        int lenWritten( assetLoader->write( tuple[_data].raw(), offset, length ) );

        response[_assetName] = assetName;
        response[_offset] = offset;
        response[_length] = lenWritten;
    }

    response[_success] = success ? 1 : 0;

    m_tupleRouter.route( response );
}

void Linda2Responder::close( const Tuple& tuple )
{
    bool success( true );

    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetCloseResponse );
    response[ _collectionName ] = m_collectionName;

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    String assetName = tuple[_assetName];
    AssetLoader* assetLoader( getAssetLoader( assetName ) );

    if( assetLoader )
    {
        success = assetLoader->close();
        deleteAssetLoader( assetName );
    }
    else
    {
        LOG_DEBUG( "Linda2AssetLoaderResponder: Error: Received close request but asset not opened" );
        success = false;
    }

    response[_success] = success ? 1 : 0;

    m_tupleRouter.route( response );
}

void Linda2Responder::move( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetMoveResponse );
    response[ _collectionName ] = m_collectionName;

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    String assetName = tuple[_assetName];
    String newName = tuple[_newName];
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, assetName ) );

    response[_success] = assetLoader->move( newName );

    delete( assetLoader );

    m_tupleRouter.route( response );
}

void Linda2Responder::erase( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _AssetLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _AssetEraseResponse );
    response[ _collectionName ] = m_collectionName;

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    String assetName = tuple[_assetName];
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, assetName ) );

    response[_success] = assetLoader->erase();

    delete( assetLoader );

    m_tupleRouter.route( response );
}

} // namespace AssetLoaders

} // namespace Agape
