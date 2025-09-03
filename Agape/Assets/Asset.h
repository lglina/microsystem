#ifndef AGAPE_ASSET_H
#define AGAPE_ASSET_H

namespace Agape
{

class AssetLoader;

class Asset
{
public:
    Asset( AssetLoader& loader );
    virtual ~Asset();

    virtual int read( char* data, int offset, int len ) const;
    virtual int write( const char* data, int offset, int len );
    virtual int size();
    virtual int readAll( char* data, int offset, int len ) const;

protected:
    AssetLoader& m_loader;
};

} // namespace Agape

#endif // AGAPE_ASSET_H
