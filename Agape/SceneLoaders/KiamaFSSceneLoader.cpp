#include "Loggers/Logger.h"
#include "Utils/EscapeBase64.h"
#include "Utils/LiteStream.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "KiamaFSSceneLoader.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace SceneLoaders
{

KiamaFS::KiamaFS( const World::Coordinates& coordinates, Agape::KiamaFS& fs, Map< String, int>& index ) :
  SceneLoader( coordinates ),
  m_fs( fs ),
  m_index( index ),
  m_inIndex( false )
{
}

bool KiamaFS::load( World::Scene& scene )
{
    scene.m_sceneItems.clear();

    LiteStream filenameStream;
    filenameStream << escapeBase64( m_coordinates.m_worldID ).substr( 0, 8 ) << "_" << m_coordinates.m_x << "_" << m_coordinates.m_y << ".scn";

    LOG_DEBUG( "Attempting to load " + filenameStream.str() + " with KiamaFS" );

    /*
    if( m_index.find( filenameStream.str() ) != m_index.end() )
    {
        m_inIndex = true;

        LOG_DEBUG( "Found in filesystem index" );

        Agape::KiamaFS::File file( m_fs.file( filenameStream.str() ) );
        if( file.open( Agape::KiamaFS::File::OpenMode::readMode ) )
        {
            Value sceneValue( Value::fromReadableWritable( file ) );
            scene = Scene::fromValue( sceneValue );
        }
        else
        {
            LOG_DEBUG( "Failed to open file" );
        }
    }
    else
    {
        LOG_DEBUG( "NOT found in filesystem index" );
    }
    */

    Agape::KiamaFS::File* file( m_fs.file( filenameStream.str() ) );
    if( file->open( Agape::KiamaFS::File::OpenMode::readMode ) )
    {
        Value sceneValue;
        Value::fromReadableWritable( *file, sceneValue );
        //LOG_DEBUG( sceneValue.dump() );
        scene = Scene::fromValue( sceneValue );

        return true;
    }

    return false;
}

bool KiamaFS::request( const Vector< SceneRequest >& requests )
{
    auto it( requests.begin() );
    for( ; it != requests.end(); ++it )
    {
        handleRequest( *it );
        m_updateLoopback.push_back( *it );
    }

    //std::copy( requests.begin(), requests.end(), std::back_inserter( m_updateLoopback ) );

    return true;
}

Vector< SceneRequest > KiamaFS::getUpdates()
{
    Vector< SceneRequest > updates;
    updates = m_updateLoopback;
    m_updateLoopback.clear();
    return updates;
}

void KiamaFS::handleRequest( const SceneRequest& request )
{
    Scene currentScene;
    load( currentScene );

    if( request.m_sceneOperation == SceneRequest::create )
    {
        currentScene.m_sceneItems.push_back( request.m_sceneItem );
    }
    else if( request.m_sceneOperation == SceneRequest::update )
    {
        Vector< SceneItem >::iterator it( currentScene.m_sceneItems.begin() );
        for( ; it != currentScene.m_sceneItems.end(); ++it )
        {
            if( *it == request.m_sceneItem )
            {
                *it = request.m_sceneItem;
                break;
            }
        }
    }
    else if( request.m_sceneOperation == SceneRequest::remove )
    {
        Vector< SceneItem >::iterator it( currentScene.m_sceneItems.begin() );
        for( ; it != currentScene.m_sceneItems.end(); ++it )
        {
            if( *it == request.m_sceneItem )
            {
                currentScene.m_sceneItems.erase( it );
                break;
            }
        }
    }

    LiteStream filenameStream;
    filenameStream << escapeBase64( m_coordinates.m_worldID ).substr( 0, 8 ) << "_" << m_coordinates.m_x << "_" << m_coordinates.m_y << ".scn";

    LOG_DEBUG( "Attempting to save " + filenameStream.str() + " with KiamaFS" );

    Agape::KiamaFS::File* file( m_fs.file( filenameStream.str() ) );
    if( file->open( Agape::KiamaFS::File::OpenMode::writeMode ) )
    {
        Value sceneValue;
        currentScene.toValue( sceneValue );
        sceneValue.toReadableWritable( *file );
        file->commit();
    }

    /*
    if( !m_inIndex )
    {
        // Add to our copy of the index.
        m_index[filenameStream.str()] = 0;
        m_inIndex = true;
    }
    */
}

} // namespace SceneLoaders

} // namespace Agape
