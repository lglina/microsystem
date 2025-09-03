#ifndef AGAPE_ASSET_LOADERS_LINDA2_H
#define AGAPE_ASSET_LOADERS_LINDA2_H

#include "Actors/NativeActors/NativeActor.h"
#include "AssetLoader.h"
#include "Promise.h"
#include "String.h"
#include "TupleRoutingCriteria.h"

using namespace Agape::Linda2;

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

namespace AssetLoaders
{

class Linda2 : public AssetLoader, public Actors::Native
{
public:
    Linda2( const World::Coordinates& coordinates,
            const String& name,
            TupleRouter& tupleRouter,
            Timers::Factory& timerFactory,
            const String& collectionName );
    ~Linda2();

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

    virtual bool move( const String& newName );
    virtual bool erase();

    virtual bool error();

    virtual bool accept( Tuple& tuple );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
    String m_collectionName;

    bool m_isLoading;

    TupleRoutingCriteria m_tupleRoutingCriteria;

    Promise m_assetOpenResponse;
    Promise m_assetReadResponse;
    Promise m_assetWriteResponse;
    Promise m_assetCloseResponse;
    Promise m_assetMoveResponse;
    Promise m_assetEraseResponse;

    bool m_isOpen;
    enum OpenMode m_openMode;
    int m_size;

    char* m_readData;
    int m_readDataLen;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_LINDA2_H
