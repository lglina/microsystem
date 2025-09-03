#include "Utils/Tokeniser.h"
#include "World/Height.h"
#include "ANSIFile.h"
#include "String.h"
#include "StringConstants.h"

#include "Utils/LiteStream.h"
#include "Loggers/Logger.h"

using namespace Agape::World;

namespace
{
    enum
    {
        defaultWidth = 80,
        defaultHeight = 25
    };

    const char* commentMetadataMagic( "AGAPE" );
} // Anonymous namespace.

namespace Agape
{

namespace Assets
{

ANSIFile::ANSIFile( AssetLoader& loader ) :
  Asset( loader )
{
    m_hasSAUCE = SAUCE::extractSAUCE( *this, m_SAUCE );
}

int ANSIFile::dataSize()
{
    if( m_hasSAUCE )
    {
        return m_SAUCE.fileSize();
    }
    else
    {
        return size();
    }
}

bool ANSIFile::hasSAUCE()
{
    return m_hasSAUCE;
}

SAUCE& ANSIFile::getSAUCE()
{
    return m_SAUCE;
}

int ANSIFile::width()
{
    if( m_hasSAUCE && m_SAUCE.tInfo1() > 0 )
    {
        return m_SAUCE.tInfo1();
    }
    else
    {
        return defaultWidth;
    }
}

int ANSIFile::height()
{
    if( m_hasSAUCE && m_SAUCE.tInfo2() > 0 )
    {
        return m_SAUCE.tInfo2();
    }
    else
    {
        return defaultHeight;
    }
}

void ANSIFile::getAssetFlags( bool& blit,
                              bool& sprite,
                              bool& animate,
                              int& frames,
                              String& templateName )
{
    blit = false;
    animate = false;
    frames = 0;

    String commentBlock( m_SAUCE.comments() );

    Tokeniser elementTokeniser( commentBlock, ';' );
    if( elementTokeniser.token() == commentMetadataMagic )
    {
        String element( elementTokeniser.token() );
        while( element != "" )
        {
            if( element.find( _blit ) != String::npos )
            {
                blit = true;
            }
            else if( element.find( _sprite ) != String::npos )
            {
                sprite = true;
            }
            else if( element.find( _animate ) != String::npos )
            {
                Tokeniser subElementTokeniser( element, ',' );
                subElementTokeniser.token(); // "animate"
                frames = ::atoi( subElementTokeniser.token().c_str() );
                if( frames )
                {
                    animate = true;
                }
            }
            else if( element.find( _carlo ) != String::npos )
            {
                Tokeniser subElementTokeniser( element, ',' );
                subElementTokeniser.token(); // "carlo"
                templateName = subElementTokeniser.token();
            }

            element = elementTokeniser.token();
        }
    }
}

void ANSIFile::getHeights( char* heightMap,
                           int mapHeight,
                           int mapWidth,
                           int rowOffset,
                           int colOffset )
{
    String commentBlock( m_SAUCE.comments() );

    Tokeniser elementTokeniser( commentBlock, ';' );
    if( elementTokeniser.token() == commentMetadataMagic )
    {
        String element( elementTokeniser.token() );
        while( !element.empty() )
        {
            Tokeniser subElementTokeniser( element, ',' );
            String heightAndRow( subElementTokeniser.token() );
            String colRange( subElementTokeniser.token() );

            if( !heightAndRow.empty() && !colRange.empty() )
            {
                String heightChar( heightAndRow.substr( 0, 1 ) );
                String rowStr( heightAndRow.substr( 1 ) );

                Tokeniser colTokeniser( colRange, '-' );
                String colStartStr( colTokeniser.token() );
                String colEndStr( colTokeniser.token() );

                if( !rowStr.empty() && !colStartStr.empty() )
                {
                    if( colEndStr.empty() ) colEndStr = colStartStr;

                    // FIXME: Still doesn't guarantee strings have valid ints.
                    // atoi() returns zero or undefined behaviour if not...
                    int row( ::atoi( rowStr.c_str() ) );
                    int colStart( ::atoi( colStartStr.c_str() ) );
                    int colEnd( ::atoi( colEndStr.c_str() ) );

                    if( ( row == 0 ) || ( colStart == 0 ) )
                    {
                        // Must have at least a row and start col.
                        element = elementTokeniser.token();
                        continue;
                    }

                    // Adjust to zero-based indicies.
                    --row;
                    --colStart;
                    --colEnd;

                    int heightInt;
                    if( heightChar == "A" )
                    {
                        heightInt = foreground;
                    }
                    else if( heightChar == "B" )
                    {
                        heightInt = ground;
                    }
                    else
                    {
                        // Probably not a height map line.
                        break;
                    }

                    for( int i = 0; i < ( colEnd - colStart + 1 ); ++i )
                    {
                        int curRow( rowOffset + row );
                        int curCol( colOffset + colStart + i );
                        if( ( curRow < mapHeight ) &&
                            ( curCol < mapWidth ) )
                        {
                            char* currentHeight( &heightMap[ ( mapWidth * curRow ) + curCol ] );
                            *currentHeight = heightInt;
                        }
                    }
                }
            }

            element = elementTokeniser.token();
        }
    }
}

void ANSIFile::setAssetFlags( bool blit,
                              bool sprite,
                              bool animate,
                              int frames,
                              const String& templateName )
{
    m_assetFlagsStr.clear();

    if( blit )
    {
        m_assetFlagsStr += String( _blit ) + ";";
    }

    if( sprite )
    {
        m_assetFlagsStr += String( _sprite ) + ";";
    }

    if( animate )
    {
        LiteStream stream;
        stream << _animate << "," << frames << ";";
        m_assetFlagsStr += stream.str();
    }

    if( !templateName.empty() )
    {
        LiteStream stream;
        stream << _carlo << "," << templateName << ";";
        m_assetFlagsStr += stream.str();
    }
}

void ANSIFile::setHeights( const char* heightMap,
                           int mapWidth,
                           int rowOffset,
                           int colOffset,
                           int height,
                           int width )
{
    // Assumption: Caller-provided bounding box is within valid limits
    // of map array.

    m_heightsStr.clear();

    for( int row = rowOffset; row < ( rowOffset + height ); ++row )
    {
        int previousHeight( 0 );
        int previousHeightStartCol( colOffset );
        int currentHeight( 0 );
        const char* mapPtr( heightMap + ( row * mapWidth ) );
        int relativeRow1Based( ( row - rowOffset ) + 1 );
        for( int col = colOffset; col < ( colOffset + width ); ++col )
        {
            currentHeight = *( mapPtr + col );
            if( currentHeight != previousHeight )
            {
                if( previousHeight != background )
                {
                    writeHeight( previousHeight,
                                 relativeRow1Based,
                                 previousHeightStartCol - colOffset + 1,
                                 col - colOffset );
                }
                previousHeight = currentHeight;
                previousHeightStartCol = col;
            }
        }

        // Write remaining height block if any outstanding.
        if( currentHeight != background )
        {
            writeHeight( currentHeight,
                         relativeRow1Based,
                         previousHeightStartCol - colOffset + 1,
                         width );
        }
    }
}

bool ANSIFile::writeSAUCE( int offset )
{
    if( !( m_assetFlagsStr.empty() && m_heightsStr.empty() ) )
    {
        String sauceComments;
        sauceComments += String( commentMetadataMagic ) + ";";
        sauceComments += m_assetFlagsStr;
        sauceComments += m_heightsStr;
        m_SAUCE.setComments( sauceComments );
    }

    return m_SAUCE.writeSAUCE( *this, offset );
}

void ANSIFile::writeHeight( int height,
                            int row1Based,
                            int startCol1Based,
                            int endCol1Based )
{
    LiteStream stream;
    if( height == foreground )
    {
        stream << "A";
    }
    else if( height == ground )
    {
        stream << "B";
    }

    stream << row1Based << "," << startCol1Based;

    if( startCol1Based != endCol1Based )
    {
        // Create range.
        stream << "-" << endCol1Based;
    }

    stream << ";";

    m_heightsStr += stream.str();
}

} // namespace Assets

} // namespace Agape
