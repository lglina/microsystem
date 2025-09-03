#ifndef AGAPE_ASSET_LOADERS_MONGO_H
#define AGAPE_ASSET_LOADERS_MONGO_H

#include "AssetLoader.h"
#include "String.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace World
{
class Coordinates;
} // namespace World

using namespace Stratus;

namespace AssetLoaders
{

class Mongo : public AssetLoader
{
public:
    Mongo( const World::Coordinates& coordinates,
           const String& name,
           const String& collectionName,
           bool encryptedNames,
           Authenticator& authenticator );

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

    virtual bool move( const String& newName );
    virtual bool erase();

private:
    String m_collectionName;
    bool m_encryptedNames;
    Authenticator& m_authenticator;

    bool m_isOpen;
    enum OpenMode m_openMode;

    String m_linkedItem;

    String m_readBuffer;
    String m_writeBuffer;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_MONGO_H
