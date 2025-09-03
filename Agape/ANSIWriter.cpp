#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/SAUCE.h"
#include "Clocks/Clock.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "ANSITerminal.h"
#include "ANSIWriter.h"
#include "String.h"

using namespace Agape::Assets;
using namespace Agape::World;

namespace
{
    const char* eol( "\r\n" );
    const int defaultForegroundColour( 0x07 );
    const int defaultBackgroundColour( 0x00 );
} // Anonymous namespace

namespace Agape
{

ANSIWriter::ANSIWriter( AssetLoaders::Factory& assetLoaderFactory,
                        const Coordinates& coordinates,
                        Clock& clock,
                        const String& assetName ) :
  m_assetLoaderFactory( assetLoaderFactory ),
  m_coordinates( coordinates ),
  m_clock( clock ),
  m_assetName( assetName )
{
}

bool ANSIWriter::write( const char* buffer,
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
                        const String& templateName )
{
    bool success( true );

    String tempName( m_assetName + "_" );

    AssetLoader* writeLoader( m_assetLoaderFactory.makeLoader( m_coordinates, tempName ) );
    if( writeLoader->open( AssetLoader::modeWrite, String() ) )
    {
        Assets::ANSIFile ansiFile( *writeLoader );

        int assetOffset( 0 );

        success = writeBuffer( ansiFile,
                               assetOffset,
                               buffer,
                               height,
                               width,
                               regionRow,
                               regionCol,
                               regionHeight,
                               regionWidth );
        
        if( success ) success = writeSAUCE( ansiFile,
                                            assetOffset,
                                            heightMap,
                                            height,
                                            width,
                                            regionRow,
                                            regionCol,
                                            regionHeight,
                                            regionWidth,
                                            sauce,
                                            blit,
                                            sprite,
                                            animFrames,
                                            templateName );

        if( success && !( writeLoader->close() && writeLoader->move( m_assetName ) ) )
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    delete( writeLoader );

    return success;
}

bool ANSIWriter::writeBuffer( ANSIFile& ansiFile,
                              int& assetOffset,
                              const char* buffer,
                              int height,
                              int width,
                              int regionRow,
                              int regionCol,
                              int regionHeight,
                              int regionWidth )
{
    // The goal in writing this was to be able to load up a file created with
    // Mœbius, to save it out and to end up with an output 1:1 *byte identical*
    // to the original input file. I.e. that we should emit escape sequences
    // in exactly the same way as Mœbius. As far as I can test this should
    // be the case, although there may be some missed edge cases!

    int rowWidthBytes( width * 2 );

    bool success( writeSGRReset( ansiFile, assetOffset ) );

    int lastRow( regionRow + regionHeight - 1 );
    for( int row = regionRow; success && ( row < ( regionRow + regionHeight ) ); ++row )
    {
        // Find last non-blank column in this row.
        int lastCol( regionCol + regionWidth - 1 );
        int lastNonBlankCol( regionCol - 1 );
        const char* bufferPtr( buffer + ( row * rowWidthBytes ) + ( lastCol * 2 ) );
        for( int col = lastCol; col >= regionCol; --col )
        {
            if( ( *bufferPtr != '\x00' ) || ( *( bufferPtr + 1 ) != 0x00 ) )
            {
                lastNonBlankCol = col;
                break;
            }
            bufferPtr -= 2;
        }

        // Write all non-blank characters in this row
        // with SGR sequences as needed.
        bufferPtr = buffer + ( row * rowWidthBytes ) + ( regionCol * 2 );
        int col( regionCol );
        for( ; success && ( col <= lastNonBlankCol ); ++col )
        {
            char c( *bufferPtr );
            unsigned char attributes( *( (const char*)( bufferPtr + 1 ) ) );
            int foregroundColour( attributes & 0x0F );
            int backgroundColour( attributes >> 4 );

            // Convert blanks into black spaces.
            if( c == '\x00' )
            {
                c = ' ';
                foregroundColour = defaultForegroundColour;
                backgroundColour = defaultBackgroundColour;
            }

            // Write SGR sequence if needed.
            if( ( foregroundColour != m_foregroundColour ) ||
                ( backgroundColour != m_backgroundColour ) )
            {
                success = writeSGRSequence( ansiFile,
                                            assetOffset,
                                            foregroundColour,
                                            backgroundColour );
            }

            // Write character.
            if( success )
            {
                int bytesWritten( ansiFile.write( &c, assetOffset, 1 ) );
                if( bytesWritten != 1 )
                {
                    success = false;
                }
                assetOffset += bytesWritten;
                bufferPtr += 2;
            }
        }

        // After the last non-blank in the line, if not already at the
        // end of the line, insert an EOL.
        if( success &&
            ( col <= lastCol ) &&
            ( col == ( lastNonBlankCol + 1 ) ) )
        {
            // Reset to default colours - this mirrors the behaviour of Mœbius,
            // which inserts this sequence at the end of *every* non-full row
            // where the colours are not already defaulted, even if it's the
            // last row in the file.
            if( ( defaultForegroundColour != m_foregroundColour ) ||
                ( defaultBackgroundColour != m_backgroundColour ) )
            {
                success = writeSGRSequence( ansiFile,
                                            assetOffset,
                                            defaultForegroundColour,
                                            defaultBackgroundColour );
            }

            if( row < lastRow )
            {
                int bytesWritten( ansiFile.write( eol, assetOffset, 2 ) );
                if( bytesWritten != 2 )
                {
                    success = false;
                }
                assetOffset += bytesWritten;
            }
        }
    }

    return success;
}

bool ANSIWriter::writeSAUCE( ANSIFile& ansiFile,
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
                             const String& templateName )
{
    sauce.setFileSize( assetOffset );
    sauce.setTInfo1( regionWidth );
    sauce.setTInfo2( regionHeight );

    // Copy SAUCE passed from ANSIEditor (either new metadata, or read from the
    // existing ANSI file) and use this to initialise the SAUCE for the file
    // we're about to write.
    SAUCE& newSauce( ansiFile.getSAUCE() );
    newSauce = sauce;

    ansiFile.setAssetFlags( blit,
                            sprite,
                            ( animFrames > 0 ),
                            animFrames,
                            templateName );
    ansiFile.setHeights( heightMap,
                         width,
                         regionRow,
                         regionCol,
                         regionHeight,
                         regionWidth );

    return ansiFile.writeSAUCE( assetOffset );
}

bool ANSIWriter::writeSGRReset( ANSIFile& ansiFile,
                                int& assetOffset )
{
    String sequence( "\x1b[0m" );
    int lenWritten( ansiFile.write( sequence.c_str(), assetOffset, sequence.length() ) );
    assetOffset += lenWritten;
    m_sgrBlink = false;
    m_sgrBold = false;
    m_foregroundColour = defaultForegroundColour;
    m_backgroundColour = defaultBackgroundColour;
    return( lenWritten == sequence.length() );
}

bool ANSIWriter::writeSGRSequence( ANSIFile& ansiFile,
                                   int& assetOffset,
                                   int foregroundColour,
                                   int backgroundColour )
{
    LiteStream sequence;

    sequence << "\x1b[";

    bool first( true );

    // To unset bold or blink, we must emit a reset.
    if( ( !( foregroundColour & 0x08 ) && m_sgrBold ) ||
        ( !( backgroundColour & 0x08 ) && m_sgrBlink ) )
    {
        sequence << "0";
        m_sgrBold = false;
        m_sgrBlink = false;
        m_foregroundColour = defaultForegroundColour;
        m_backgroundColour = defaultBackgroundColour;
        first = false;
    }

    if( ( foregroundColour & 0x08 ) && !m_sgrBold )
    {
        if( !first ) sequence << ";"; else first = false;
        sequence << "1";
        m_sgrBold = true;
    }

    if( ( backgroundColour & 0x08 ) && !m_sgrBlink )
    {
        if( !first ) sequence << ";"; else first = false;
        sequence << "5";
        m_sgrBlink = true;
    }

    if( foregroundColour != m_foregroundColour )
    {
        if( !first ) sequence << ";"; else first = false;
        m_foregroundColour = foregroundColour;
        foregroundColour = ANSITerminal::fromTermColour( foregroundColour );
        foregroundColour &= 0x07;
        sequence << ( 30 + foregroundColour );
    }

    if( backgroundColour != m_backgroundColour )
    {
        if( !first ) sequence << ";"; else first = false;
        m_backgroundColour = backgroundColour;
        backgroundColour = ANSITerminal::fromTermColour( backgroundColour );
        backgroundColour &= 0x07;
        sequence << ( 40 + backgroundColour );
    }

    sequence << "m";

    String sequenceStr( sequence.str() );
    int bytesWritten( ansiFile.write( sequenceStr.c_str(), assetOffset, sequenceStr.length() ) );
    assetOffset += bytesWritten;
    return( bytesWritten == sequenceStr.length() );
}

} // namespace Agape
