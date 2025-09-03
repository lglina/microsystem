#include "Loggers/Logger.h"
#include "Utils/EscapeBase64.h"
#include "Utils/LiteStream.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "FileSceneLoader.h"
#include "FileWriter.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "Value.h"

#include <unistd.h>

namespace Agape
{

namespace SceneLoaders
{

File::File( const World::Coordinates& coordinates,
            const String& scenePath,
            const String& sceneExtension,
            const String& attributesPath,
            const String& attributesExtension ) :
  SceneLoader( coordinates ),
  m_scenePath( scenePath ),
  m_sceneExtension( sceneExtension ),
  m_attributesPath( attributesPath ),
  m_attributesExtension( attributesExtension )
{
}

bool File::load( World::Scene& scene )
{
    scene.m_sceneItems.clear();

    FileWriter fileWriter( sceneFilename(), FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value sceneValue;
        if( Value::fromReadableWritable( fileWriter, sceneValue ) )
        {
            scene = Scene::fromValue( sceneValue );
            return true;
        }
    }

    return false;
}

bool File::request( const Vector< SceneRequest >& requests )
{
    auto it( requests.begin() );
    for( ; it != requests.end(); ++it )
    {
        handleRequest( *it );
        m_updateLoopback.push_back( *it );
    }

    return true;
}

Vector< SceneRequest > File::getUpdates()
{
    Vector< SceneRequest > updates;
    updates = m_updateLoopback;
    m_updateLoopback.clear();
    return updates;
}

bool File::hasSceneItemAttribute( const String& snowflake,
                                  const String& name )
{
    Value attributes;
    return( loadSceneItemAttributesFile( snowflake, attributes ) && attributes.hasValue( name ) );
}

bool File::createSceneItemAttribute( const String& snowflake,
                                     const String& name )
{
    Value attributes;
    if( loadSceneItemAttributesFile( snowflake, attributes ) )
    {
        attributes[name] = Value();
        return( saveSceneItemAttributesFile( snowflake, attributes ) );
    }
    
    return false;
}

bool File::loadSceneItemAttribute( const String& snowflake,
                                   const String& name,
                                   Value& value )
{
    Value attributes;
    if( loadSceneItemAttributesFile( snowflake, attributes ) )
    {
        if( attributes.hasValue( name ) )
        {
            value = attributes[name];
            return true;
        }
    }

    return false;
}

bool File::saveSceneItemAttribute( const String& snowflake,
                                   const String& name,
                                   const Value& value )
{
    Value attributes;
    if( loadSceneItemAttributesFile( snowflake, attributes ) )
    {
        attributes[name] = value;
        return( saveSceneItemAttributesFile( snowflake, attributes ) );
    }

    return false;
}

bool File::deleteSceneItemAttributes( const String& snowflake )
{
    return( ::unlink( sceneItemAttributesFilename( snowflake ).c_str() ) == 0 );
}

void File::save( const World::Scene& scene )
{
    Value sceneValue;
    scene.toValue( sceneValue );

    LiteStream filenameStream;
    filenameStream << escapeBase64( m_coordinates.m_worldID ).substr( 0, 8 ) << "_" << m_coordinates.m_x << "_" << m_coordinates.m_y << ".scn";

    FileWriter fileWriter( sceneFilename(), FileWriter::modeWrite );

    sceneValue.toReadableWritable( fileWriter );
}

bool File::loadSceneItemAttributesFile( const String& snowflake, Value& value )
{
    FileWriter fileWriter( sceneItemAttributesFilename( snowflake ), FileWriter::modeRead );
    if( fileWriter.isOpen() )
    {
        return( Value::fromReadableWritable( fileWriter, value ) );
    }

    return false;
}

bool File::saveSceneItemAttributesFile( const String& snowflake, const Value& attributes )
{
    FileWriter fileWriter( sceneItemAttributesFilename( snowflake ), FileWriter::modeWrite );
    if( fileWriter.isOpen() )
    {
        attributes.toReadableWritable( fileWriter );
        return true;
    }

    return false;
}

void File::handleRequest( const SceneRequest& request )
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

    save( currentScene );
}

String File::sceneFilename()
{
    LiteStream filenameStream;
    filenameStream << m_scenePath << "/"
                   << escapeBase64( m_coordinates.m_worldID ).substr( 0, 8 ) << "_"
                   << m_coordinates.m_x << "_"
                   << m_coordinates.m_y << "."
                   << m_sceneExtension;
    return filenameStream.str();
}

String File::sceneItemAttributesFilename( const String& snowflake )
{
    LiteStream filenameStream;
    filenameStream << m_attributesPath << "/"
                   << snowflake << "."
                   << m_attributesExtension;
    return filenameStream.str();
}

} // namespace SceneLoaders

} // namespace Agape
