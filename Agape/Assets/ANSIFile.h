#ifndef AGAPE_ASSETS_ANSI_FILE_H
#define AGAPE_ASSETS_ANSI_FILE_H

#include "Asset.h"
#include "SAUCE.h"
#include "String.h"

namespace Agape
{

class AssetLoader;

namespace Assets
{

class ANSIFile : public Asset
{
public:
    ANSIFile( AssetLoader& loader );

    /// Returns size of ANSI file according to SAUCE metadata, if SAUCE
    /// present, or the entire file size if no SAUCE.
    int dataSize();

    bool hasSAUCE();
    SAUCE& getSAUCE();

    int width();
    int height();

    void getAssetFlags( bool& blit,
                        bool& sprite,
                        bool& animate,
                        int& frames,
                        String& templateName );
    void getHeights( char* heightMap,
                     int mapHeight,
                     int mapWidth,
                     int rowOffset,
                     int colOffset );

    void setAssetFlags( bool blit,
                        bool sprite,
                        bool animate,
                        int frames,
                        const String& templateName );
    void setHeights( const char* heightMap,
                     int mapWidth,
                     int rowOffset,
                     int colOffset,
                     int height,
                     int width );

    bool writeSAUCE( int offset );

private:
    void writeHeight( int height,
                      int row1Based,
                      int startCol1Based,
                      int endCol1Based );

    bool m_hasSAUCE;
    SAUCE m_SAUCE;

    String m_assetFlagsStr;
    String m_heightsStr;
};

} // namespace Assets

} // namespace Agape

#endif // AGAPE_ASSETS_ANSI_FILE_H
