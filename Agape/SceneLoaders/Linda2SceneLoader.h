#ifndef AGAPE_SCENE_LOADERS_LINDA2_H
#define AGAPE_SCENE_LOADERS_LINDA2_H

#include "Actors/NativeActors/NativeActor.h"
#include "Collections.h"
#include "Promise.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "TupleRoutingCriteria.h"
#include "Value.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
} // namespace World

class Timer;

namespace SceneLoaders
{

class Linda2 : public SceneLoader, public Actors::Native
{   
public:
    Linda2( const Coordinates& coordinates,
            bool receiveRequests,
            TupleRouter& tupleRouter,
            Timers::Factory& timerFactory,
            bool encrypted );
    virtual ~Linda2();

    virtual bool load( Scene& scene );
    virtual void save( const Scene& scene );
    virtual bool request( const Vector< SceneRequest >& requests );
    virtual Vector< SceneRequest > getUpdates();

    virtual bool overflowed();

    virtual bool hasSceneItemAttribute( const String& snowflake,
                                        const String& name );
    virtual bool createSceneItemAttribute( const String& snowflake,
                                           const String& name );
    virtual bool loadSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         Value& value );
    virtual bool saveSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         const Value& value );
    virtual bool deleteSceneItemAttributes( const String& snowflake );

    virtual void invalidateCachedAsset( const struct InvalidatedAsset& invalidatedAsset );
    virtual Vector< struct InvalidatedAsset > getInvalidatedAssets();

    virtual bool accept( Tuple& tuple );

private:
    void saveToCache( const String& snowflake, const String& name, const Value& value );
    bool loadFromCache( const String& snowflake, const String& name, Value& value );
    void deleteFromCache( const String& snowflake );
    void purgeCache();

    bool m_receiveRequests;
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
    bool m_encrypted;

    bool m_isLoading;
    int m_currentItem;
    int m_totalItems;

    TupleRoutingCriteria m_sceneLoadResponseRoutingCriteria;
    TupleRoutingCriteria m_sceneLoadResponseTransportRoutingCriteria;
    TupleRoutingCriteria m_sceneItemSaveAttributeRoutingCriteria;
    TupleRoutingCriteria m_sceneItemDeleteAttributesRoutingCriteria;

    Scene* m_currentScene;

    Vector< SceneRequest > m_updates;

    bool m_isSavingAttribute;
    bool m_isDeletingAttributes;
    String m_saveDeleteSnowflake;
    String m_saveAttributeName;

    Promise m_sceneLoadResponse;
    Promise m_sceneItemCreateAttributeResponse;
    Promise m_sceneItemLoadAttributeResponse;
    Promise m_sceneItemSaveAttributeResponse;
    Promise m_sceneItemDeleteAttributesResponse;

    bool m_overflowed;

    Map< String, Value* > m_cachedAttributes;

    Vector< struct InvalidatedAsset > m_invalidatedAssets;
    TupleRoutingCriteria m_invalidateCachedAssetRoutingCriteria;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_LINDA2_H
