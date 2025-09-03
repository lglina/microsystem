#include "Collections.h"
#include "Scene.h"
#include "SceneItem.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "WorldCoordinates.h"

namespace
{
    const int _maxItems( 64 );
} // Anonymous namespace

namespace Agape
{

namespace World
{

Scene::Scene()
{
    m_sceneItems.reserve( _maxItems );
}

void Scene::toValue( Value& value ) const
{
    Value& sceneItems( value[_sceneItems] );
    Vector< SceneItem >::const_iterator it( m_sceneItems.begin() );
    for( ; it != m_sceneItems.end(); ++it )
    {
        Value* sceneItem( new Value );
        it->toValue( *sceneItem );
        sceneItems.push_back( sceneItem );
    }

    m_coordinates.toValue( value[_coordinates] );
}

Scene Scene::fromValue( const Value& value )
{
    Scene scene;
    const Value& sceneItems( value[_sceneItems] );
    ConstListIterator it( sceneItems.listBegin() );
    for( ; it != sceneItems.listEnd() && scene.m_sceneItems.size() < _maxItems; ++it )
    {
        scene.m_sceneItems.push_back( SceneItem::fromValue( **it ) );
    }

    scene.m_coordinates = Coordinates::fromValue( value[_coordinates] );

    return scene;
}

int Scene::maxItems() const
{
    return _maxItems;
}

bool Scene::encrypt( Encryptor& encryptor )
{
    Vector< SceneItem >::iterator it( m_sceneItems.begin() );
    for( ; it != m_sceneItems.end(); ++it )
    {
        if( !it->encrypt( encryptor ) ) return false;
    }

    return true;
}

bool Scene::decrypt( Encryptor& encryptor )
{
    Vector< SceneItem >::iterator it( m_sceneItems.begin() );
    for( ; it != m_sceneItems.end(); ++it )
    {
        if( !it->decrypt( encryptor ) ) return false;
    }

    return true;
}

} // namespace World

} // namespace Agape
