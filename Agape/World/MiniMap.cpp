#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/Factories/BakedAssetLoaderFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "GraphicsDrivers/Headless.h"
#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "SceneLoaders/SceneLoader.h"
#include "Utils/Tokeniser.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "ANSITerminal.h"
#include "MiniMap.h"
#include "String.h"
#include "StringConstants.h"

#include <math.h>
#include <string.h>

namespace
{
    const int bufferSize = 512;
} // Anonymous namespace

namespace Agape
{

namespace World
{

MiniMap::MiniMap( SceneLoaders::Factory& sceneLoaderFactory,
                  AssetLoaders::Factory& assetLoaderFactory,
                  ANSITerminal& renderTerminal ) :
  m_sceneLoaderFactory( sceneLoaderFactory ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_renderTerminal( renderTerminal ),
  m_renderSize( 0 ),
  m_fastMode( true )
{
    m_buffer = new char[bufferSize];
}

MiniMap::~MiniMap()
{
    delete[]( m_buffer );
}

void MiniMap::render( const World::Coordinates& coordinates, int xtiles, int ytiles )
{
    m_renderTerminal.clearScreen();

    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates, SceneLoader::noReceiveRequests ) );
    Scene scene;
    sceneLoader->load( scene );
    delete( sceneLoader );
    tileBackground( coordinates );

    Vector< SceneItem >::const_iterator iter;
    for( iter = scene.m_sceneItems.begin(); iter != scene.m_sceneItems.end(); ++iter )
    {
        const SceneItem& thisSceneItem( *iter );

        if( !m_fastMode )
        {
            // Slow mode: Open and draw every asset to render terminal.
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, thisSceneItem.assetName() ) );
            if( !assetLoader->open() )
            {
                delete( assetLoader );
                assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), _unknown );
                assetLoader->open();
            }

            Assets::ANSIFile ansiFile( *assetLoader );

            bool blit( false );
            bool sprite( false );
            bool animate( false );
            int frames( 0 );
            String templateName;
            if( ansiFile.hasSAUCE() )
            {
                ansiFile.getAssetFlags( blit, sprite, animate, frames, templateName );
            }

            if( blit || sprite ||
                ( animate && ( frames > 0 ) && ( ( ansiFile.height() % frames ) == 0 ) ) )
            {
                m_renderTerminal.createSprite( thisSceneItem.snowflake(),
                                            String(),
                                            ansiFile,
                                            ansiFile.dataSize(),
                                            thisSceneItem.row(),
                                            thisSceneItem.col(),
                                            ansiFile.height(),
                                            ansiFile.width(),
                                            Terminal::whitespaceTransparency | ( blit ? Terminal::blit : 0 ),
                                            animate ? frames : 1 );
            }
            else
            {
                m_renderTerminal.consumeNext( thisSceneItem.row(), thisSceneItem.col() );
                m_renderTerminal.consumeAsset( ansiFile,
                                            0, // Offset zero
                                            ansiFile.dataSize(),
                                            ansiFile.width(),
                                            thisSceneItem.col(),
                                            Terminal::noMaxRow,
                                            Terminal::scrollLock,
                                            Terminal::whitespaceTransparency );
            }

            delete( assetLoader );
        }
        else
        {
            // Fast mode: Draw every item as its bounding box with
            // a solid colour.
            for( int y = thisSceneItem.row(); y < thisSceneItem.row() + thisSceneItem.height(); ++y )
            {
                for( int x = thisSceneItem.col(); x < thisSceneItem.col() + thisSceneItem.width(); ++x )
                {
                    m_renderTerminal.consumeNext( y, x, 0x07 );
                    m_renderTerminal.consumeChar( '\xdb', Terminal::scrollLock );
                }
            }
        }
    }

    // Shrink down rendered scene and create ANSI output.
    int currentTileAtt( 0 );
    char* buffPtr( m_buffer );
    for( int startRow = 0; startRow <= ( m_renderTerminal.height() - ytiles ); startRow += ytiles )
    {
        for( int startCol = 0; startCol <= ( m_renderTerminal.width() - xtiles ); startCol += xtiles )
        {
            char character;
            char attribute;
            double red = 0;
            double green = 0;
            double blue = 0;
            for( int tileRow = 0; tileRow < ytiles; ++tileRow )
            {
                for( int tileCol = 0; tileCol < xtiles; ++tileCol )
                {
                    m_renderTerminal.getBaseCharAt( startRow + tileRow, startCol + tileCol, character, attribute );
                    unsigned char uattr( *( (unsigned char*)&attribute ) );
                    unsigned char bg( uattr >> 4 );
                    unsigned char fg( uattr & 0x0F );

                    unsigned char maincol = 0;
                    int cred = 0;
                    int cblue = 0;
                    int cgreen = 0;
                    if( character == '\0' || character == ' ' )
                    {
                        maincol = bg;
                    }
                    else
                    {
                        maincol = fg;
                    }

                    cred = ( maincol & 0x04 ) >> 2;
                    cgreen = ( maincol & 0x02 ) >> 1;
                    cblue = ( maincol & 0x01 );

                    if( maincol & 0x08 )
                    {
                        cred *= 2;
                        cblue *= 2;
                        cgreen *= 2;
                    }

                    red += cred;
                    green += cgreen;
                    blue += cblue;
                }
            }

            red /= ( xtiles * ytiles );
            green /= ( xtiles * ytiles * 1.5 ); // De-emphasise grass so that smaller objects appear at low zoom levels.
            blue /= ( xtiles * ytiles );

            double sum = ( red + green + blue ) / 3;

            char c( '\0' );
            int intens( 0 );
            if( sum <= 0.25 )
            {
                c = '\xb0';
            }
            else if( sum <= 0.5 )
            {
                c = '\xb1';
            }
            else if( sum <= 0.75 )
            {
                c = '\xb2';
            }
            else if( sum <= 1 )
            {
                c = '\xdb';
            }
            else if( sum <= 1.25 )
            {
                c = '\xb0';
                intens = 1;
            }
            else if( sum <= 1.5 )
            {
                c = '\xb1';
                intens = 1;
            }
            else if( sum <= 1.75 )
            {
                c = '\xb2';
                intens = 1;
            }
            else
            {
                c = '\xdb';
                intens = 1;
            }

            int ired = ::round( red );
            int igreen = ::round( green );
            int iblue = ::round( blue );

            if( ired > 1 ) ired = 1;
            if( igreen > 1 ) igreen = 1;
            if( iblue > 1 ) iblue = 1;

            int tileAtt = ( intens << 3 ) +
                          ( ired << 2 ) +
                          ( igreen << 1 ) +
                          iblue;
            
            if( tileAtt != currentTileAtt )
            {
                String colourEscape( ANSITerminal::colour( tileAtt ) );
                if( ( ( buffPtr - m_buffer ) + colourEscape.length() ) <= bufferSize ) // Check overflow.
                {
                    ::memcpy( buffPtr, colourEscape.c_str(), colourEscape.length() );
                    buffPtr += colourEscape.length();
                }

                currentTileAtt = tileAtt;
            }

            if( ( ( buffPtr - m_buffer ) + 1 ) <= bufferSize ) // Check overflow.
            {
                *buffPtr++ = c;
            }
        }

        if( ( ( buffPtr - m_buffer ) + 2 ) <= bufferSize ) // Check overflow.
        {
            *buffPtr++ = '\r';
            *buffPtr++ = '\n';
        }
    }

    m_renderSize = buffPtr - m_buffer;
}

int MiniMap::renderedSize()
{
    return m_renderSize;
}

int MiniMap::read( char* data, int offset, int len )
{
    int lenToRead( len > ( m_renderSize - offset ) ? ( m_renderSize - offset ) : len );
    ::memcpy( data, m_buffer + offset, lenToRead );
    return lenToRead;
}

void MiniMap::tileBackground( const World::Coordinates& coordinates )
{
    if( !m_fastMode )
    {
        // Slow mode: Tile background asset.
        AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, _ground ) );
        if( !assetLoader->open() )
        {
            delete( assetLoader );
            assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), _unknown );
            assetLoader->open();
        }

        Assets::ANSIFile ansiFile( *assetLoader );

        int tileWidth = ansiFile.width();
        int tileHeight = ansiFile.height();
        for( int row = 0; row < m_renderTerminal.height(); row += tileHeight )
        {
            for( int col = 0; col < m_renderTerminal.width(); col += tileWidth )
            {
                m_renderTerminal.consumeNext( row, col );
                m_renderTerminal.consumeAsset( ansiFile, 0, ansiFile.dataSize(), tileWidth, col, Terminal::noMaxRow, Terminal::scrollLock );
            }
        }

        delete( assetLoader );
    }
    else
    {
        // Fast mode: Draw background as solid green.
        for( int y = 0; y < m_renderTerminal.height(); ++y )
        {
            for( int x = 0; x < m_renderTerminal.width(); ++x )
            {
                m_renderTerminal.consumeNext( y, x, 0x02 );
                m_renderTerminal.consumeChar( '\xb1', Terminal::scrollLock );
            }
        }
    }
}

} // namespace World

} // namespace Agape
