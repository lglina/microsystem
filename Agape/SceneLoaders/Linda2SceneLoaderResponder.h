#ifndef AGAPE_SCENE_LOADERS_LINDA2_RESPONDER_H
#define AGAPE_SCENE_LOADERS_LINDA2_RESPONDER_H

#include "Actors/NativeActors/NativeActor.h"

using namespace Agape::Linda2;

namespace Agape
{

class SceneRequest;

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace World
{
    class Scene;
    class SceneItem;
} // namespace World

using namespace World;

namespace SceneLoaders
{

class Factory;

class Linda2Responder : public Actors::Native
{
public:
    Linda2Responder( TupleRouter& tupleRouter, SceneLoaders::Factory& sceneLoaderFactory );
    virtual ~Linda2Responder();

    virtual bool accept( Tuple& tuple );

    void reset();

private:
    void loadScene( const Tuple& tuple );
    
    void handleRequest( const Tuple& tuple );

    void handleCreateAttributeRequest( const Tuple& tuple );
    void handleLoadAttributeRequest( const Tuple& tuple );
    void handleSaveAttributeRequest( const Tuple& tuple );
    void handleDeleteAttributesRequest( const Tuple& tuple );

    TupleRouter& m_tupleRouter;
    SceneLoaders::Factory& m_sceneLoaderFactory;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_LINDA2_RESPONDER_H
