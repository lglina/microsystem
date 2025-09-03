#ifndef AGAPE_ASSET_LOADERS_LINDA2_RESPONDER_H
#define AGAPE_ASSET_LOADERS_LINDA2_RESPONDER_H

#include "Actors/NativeActors/NativeActor.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Collections.h"
#include "String.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class TupleRouter;
class Tuple;
} // namespace Linda2

class AssetLoader;

namespace AssetLoaders
{

class Linda2Responder : public Actors::Native
{
public:
    Linda2Responder( TupleRouter& tupleRouter,
                     AssetLoaders::Factory& assetLoaderFactory,
                     const String& collectionName );
    ~Linda2Responder();

    virtual bool accept( Tuple& tuple );

    void reset();

private:
    AssetLoader* createAssetLoader( const World::Coordinates& coordinates, const String& assetName );
    AssetLoader* getAssetLoader( const String& assetName );
    void deleteAssetLoader( const String& assetName );

    void open( const Tuple& tuple );
    void read( const Tuple& tuple );
    void write( const Tuple& tuple );
    void close( const Tuple& tuple );
    void move( const Tuple& tuple );
    void erase( const Tuple& tuple );

    TupleRouter& m_tupleRouter;
    AssetLoaders::Factory& m_assetLoaderFactory;
    String m_collectionName;

    Map< String, AssetLoader* > m_assetLoaders;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_LINDA2_RESPONDER_H
