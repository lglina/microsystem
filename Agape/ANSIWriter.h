#ifndef AGAPE_ANSI_WRITER_H
#define AGAPE_ANSI_WRITER_H

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Assets
{
class ANSIFile;
class SAUCE;
} // namespace Assets

namespace World
{
class Coordinates;
} // namespace World

class Clock;
class String;
class Terminal;

using namespace Assets;
using namespace World;

class ANSIWriter
{
public:
    ANSIWriter( AssetLoaders::Factory& assetLoaderFactory,
                const Coordinates& coordinates,
                Clock& clock,
                const String& assetName );

    bool write( const char* buffer,
                const char* heightMap,
                int height,
                int width,
                int regionRow,
                int regionCol,
                int regionHeight,
                int regionWidth,
                SAUCE& sauce,
                bool blit,
                bool sprite,
                int animFrames,
                const String& templateName );

private:
    bool writeBuffer( ANSIFile& ansiFile,
                      int& assetOffset,
                      const char* buffer,
                      int height,
                      int width,
                      int regionRow,
                      int regionCol,
                      int regionHeight,
                      int regionWidth );
    bool writeSAUCE( ANSIFile& ansiFile,
                     int& assetOffset,
                     const char* heightMap,
                     int height,
                     int width,
                     int regionRow,
                     int regionCol,
                     int regionHeight,
                     int regionWidth,
                     SAUCE& sauce,
                     bool blit,
                     bool sprite,
                     int animFrames,
                     const String& templateName );

    bool writeSGRReset( ANSIFile& ansiFile,
                        int& assetOffset );
    bool writeSGRSequence( ANSIFile& ansiFile,
                           int& assetOffset,
                           int foregroundColour,
                           int backgroundColour );

    AssetLoaders::Factory& m_assetLoaderFactory;
    const Coordinates& m_coordinates;
    Clock& m_clock;
    String m_assetName;

    int m_foregroundColour;
    int m_backgroundColour;
    bool m_sgrBold;
    bool m_sgrBlink;
};

} // namespace Agape

#endif // AGAPE_ANSI_WRITER_H
