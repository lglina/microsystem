#include "Editor.h"

#include "EditorChunk.h"
#include "Terminal.h"

#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Highlighters/Factories/HighlighterFactory.h"
#include "Highlighters/Highlighter.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "String.h"

//#include "Loggers/Logger.h"

//#include <iostream>

using namespace Agape::InputDevices;

namespace
{
    const int bufferSize( 256 );
    const int idleTimeout( 5000 ); // ms.

    const int defaultBackground( Agape::Terminal::colBlue );
    const int defaultForeground( Agape::Terminal::colGrey );
} // Anonymous namespace

namespace Agape
{

namespace Editor
{

Editor::Editor( Terminal& terminal,
                AssetLoaders::Factory& assetLoaderFactory,
                Highlighters::Factory* highlighterFactory,
                const World::Coordinates& coordinates,
                const String& assetName,
                const String& instanceName,
                const String& displayName,
                const String& linkedItem,
                const String& keyGuide,
                Timers::Factory& timerFactory,
                Terminal* errorsTerminal,
                Terminal* debugLinesTerminal,
                Terminal* debugChunksTerminal ) :
  m_terminal( terminal ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_highlighterFactory( highlighterFactory ),
  m_coordinates( coordinates ),
  m_assetName( assetName ),
  m_instanceName( instanceName ),
  m_displayName( displayName ),
  m_linkedItem( linkedItem ),
  m_keyGuide( keyGuide ),
  m_errorsTerminal( errorsTerminal ),
  m_debugLinesTerminal( debugLinesTerminal ),
  m_debugChunksTerminal( debugChunksTerminal ),
  m_idleTimer( timerFactory.makeTimer() ),
  m_needsHighlight( false ),
  m_needsRedraw( false ),
  m_openMode( modeNone ),
  m_modified( false ),
  m_error( false ),
  m_assetLoader( nullptr ),
  m_unixEOL( true ),
  m_pageTopLine( m_lines.end() ),
  m_cursorRow( 0 ),
  m_cursorCol( 0 )
{
}

Editor::~Editor()
{
    delete( m_idleTimer );
    delete( m_assetLoader );
}

bool Editor::open( int openMode )
{
    m_openMode = openMode;

    m_assetLoader = m_assetLoaderFactory.makeLoader( m_coordinates, m_assetName );

    if( m_assetLoader->open( AssetLoader::modeRead, String() ) )
    {
        // Create single file chunk for entire file contents.
        Chunk chunk;
        chunk.m_type = Chunk::file;
        chunk.m_offset = 0;
        chunk.m_len = m_assetLoader->size();

        m_chunks.push_back( chunk );

        m_terminal.createCursor( "Editor", 0, 0, '_', 0x0F );

        return true;
    }
    else
    {
        if( openMode & modeCreate )
        {
            // Create initial inserted chunk with zero length.
            Chunk chunk;
            chunk.m_type = Chunk::inserted;
            chunk.m_offset = -1;
            chunk.m_len = 0;

            m_chunks.push_back( chunk );

            m_terminal.createCursor( "Editor", 0, 0, '_', 0x0F );

            //m_modified = true;

            return true;
        }
    }

    //std::cout << "Failed to open file." << std::endl;
    return false;
}

bool Editor::modified() const
{
    return m_modified;
}

bool Editor::close( bool save )
{
    return close( save, String() );
}

bool Editor::close( bool save, const String& as )
{
    bool success( true );

    String saveName( as.empty() ? m_assetName : as );

    if( ( m_openMode & modeWrite ) &&
          save &&
          !m_error )
    {
        String tempName( saveName + "_" );

        AssetLoader* writeLoader( m_assetLoaderFactory.makeLoader( m_coordinates, tempName ) );
        if( writeLoader->open( AssetLoader::modeWrite, m_linkedItem ) )
        {
            String buffer;
            int readOffset( 0 );
            int writeOffset( 0 );
            // Chunk merge logic is in read() - just keep calling here until we get
            // a return of zero (end of chunks).
            bool error( false );
            int numDidRead( read( buffer, readOffset, bufferSize, error ) );
            if( error ) success = false;
            readOffset += numDidRead;
            while( success && ( numDidRead > 0 ) )
            {
                if( writeLoader->write( &buffer[0], writeOffset, numDidRead ) == numDidRead )
                {
                    writeOffset += numDidRead;
                    numDidRead = read( buffer, readOffset, bufferSize, error );
                    if( error ) success = false;
                    readOffset += numDidRead;
                }
                else
                {
                    success = false;
                }
            }

            if( success && !( m_assetLoader->close() && writeLoader->close() && writeLoader->move( saveName ) ) )
            {
                success = false;
            }
        }
        else
        {
            success = false;
        }

        delete( writeLoader );
    }
    else
    {
        success = m_assetLoader->close();
    }

    m_terminal.deleteCursor( "Editor" );

    return success;
}

void Editor::getPosition( int& offset, int& row, int& col )
{
    offset = m_pageTopLine->m_offset;
    row = m_cursorRow;
    col = m_cursorCol;
}

bool Editor::setPosition( int offset, int row, int col )
{
    paginate();
    List< Line >::iterator it( m_lines.begin() );
    for( ; it != m_lines.end(); ++it )
    {
        if( it->m_offset >= offset )
        {
            m_pageTopLine = it;
            break;
        }
    }

    if( it != m_lines.end() )
    {
        m_cursorRow = row;
        m_cursorCol = col;

        m_terminal.moveCursor( "Editor", m_cursorRow, m_cursorCol );

        return true;
    }

    return false;
}

void Editor::draw( bool highlight, bool repaginate )
{
    if( repaginate )
    {
        paginate();
    }
    _draw( highlight, true );
}

void Editor::consumeCharacter( char c )
{
    int deleteOffset( 0 );
    int insertOffset( 0 );
    if( c == Key::right )
    {
        if( !tryMoveCursor( m_cursorRow, m_cursorCol + 1 ) )
        {
            if( m_cursorRow < ( m_terminal.height() - 2 ) )
            {
                tryMoveCursor( m_cursorRow + 1, 0 ); // Start of next line.
            }
            else if( scrollDown() )
            {
                draw( false, false );
                tryMoveCursor( m_cursorRow, 0 ); // Start of next line.
            }
        }
        debugDrawChunks(); // Update displayed cursor pos and offset
    }
    else if( c == Key::left )
    {
        if( !tryMoveCursor( m_cursorRow, m_cursorCol - 1 ) )
        {
            if( m_cursorRow > 0 )
            {
                tryMoveCursor( m_cursorRow - 1, m_terminal.width() - 1 ); // End of prev. line.
            }
            else if( scrollUp() )
            {
                draw( false, false );
                tryMoveCursor( m_cursorRow, m_terminal.width() - 1 ); // End of prev. line.
            }
        }
        debugDrawChunks(); // Update displayed cursor pos and offset
    }
    else if( c == control( Key::right ) )
    {
        tryMoveCursor( m_cursorRow, m_terminal.width() - 1 );
        debugDrawChunks(); // Update displayed cursor pos and offset
    }
    else if( c == control( Key::left ) )
    {
        tryMoveCursor( m_cursorRow, 0 );
        debugDrawChunks(); // Update displayed cursor pos and offset
    }
    else if( c == Key::down )
    {
        if( m_cursorRow < ( m_terminal.height() - 2 ) )
        {
            tryMoveCursor( m_cursorRow + 1, m_cursorCol );
            debugDrawChunks(); // Update displayed cursor pos and offset
        }
        else
        {
            if( scrollDown() )
            {
                m_terminal.scrollScreen( Terminal::scrollDown, 0, m_terminal.height() - 1 );
                m_terminal.fillScreen( '\0',
                                       Terminal::attributes( defaultBackground, defaultForeground ),
                                       m_terminal.height() - 2,
                                       1 );
                // FIXME: We could clean this up by changing _draw() to allow
                // partial screen re-draw, then just do the bottom line.
                // Ditto for scrolling the other way. Of course, it would be
                // even better to use some off-screen buffer to make scrolling
                // even faster...
                List< Line >::iterator line( getLineByRow( m_terminal.height() - 2 ) );
                String lineStr;
                bool error( false );
                read( lineStr, line->m_offset, line->m_length, error );
                if( error ) m_error = true; // Flag errors to caller.
                m_terminal.consumeNext( m_terminal.height() - 2, 0 );
                m_terminal.consumeString( lineStr, Terminal::scrollLock, Terminal::preserveBackground );
                tryMoveCursor( m_cursorRow, m_cursorCol );
                debugDrawChunks(); // Update displayed cursor pos and offset
                m_needsRedraw = true;
            }
        }
    }
    else if( c == Key::up )
    {
        if( m_cursorRow > 0 )
        {
            tryMoveCursor( m_cursorRow - 1, m_cursorCol );
            debugDrawChunks(); // Update displayed cursor pos and offset
        }
        else
        {
            if( scrollUp() )
            {
                m_terminal.scrollScreen( Terminal::scrollUp, 0, m_terminal.height() - 1 );
                m_terminal.fillScreen( '\0',
                                       Terminal::attributes( defaultBackground, defaultForeground ),
                                       0,
                                       1 );
                List< Line >::iterator line( getLineByRow( 0 ) );
                String lineStr;
                bool error( false );
                read( lineStr, line->m_offset, line->m_length, error );
                if( error ) m_error = true; // Flag errors to caller.
                m_terminal.consumeNext( 0, 0 );
                m_terminal.consumeString( lineStr, Terminal::scrollLock, Terminal::preserveBackground );
                tryMoveCursor( m_cursorRow, m_cursorCol );
                debugDrawChunks(); // Update displayed cursor pos and offset
                m_needsRedraw = true;
            }
        }
    }
    else if( c == Key::backspace )
    {
        if( m_openMode & modeWrite )
        {
            deleteOffset = ( getOffsetAtCursor() - 1 );
            if( deleteOffset >= 0 )
            {
                deleteCharacter( true );
                if( !fastBackspace() )
                {
                    paginate();
                    getCursorPositionForOffset( deleteOffset, m_cursorRow, m_cursorCol );
                    if( m_cursorRow == -1 )
                    {
                        // Offset off top of screen.
                        scrollUp();
                        getCursorPositionForOffset( deleteOffset, m_cursorRow, m_cursorCol );
                    }
                    draw( false, false );
                }
                m_terminal.moveCursor( "Editor", m_cursorRow, m_cursorCol );
            }
        }
    }
    else if( c == Key::tab )
    {
        if( m_openMode & modeWrite )
        {
            insertOffset = ( getOffsetAtCursor() + 4 );
            bool needRedraw( false );
            for( int i = 0; i < 4; ++i )
            {
                insertCharacter( ' ' );
                if( !fastInsert( ' ' ) ) needRedraw = true;
            }
            if( needRedraw )
            {
                paginate();
                getCursorPositionForOffset( insertOffset, m_cursorRow, m_cursorCol );
                if( m_cursorRow == -1 )
                {
                    // Offset off bottom of screen.
                    scrollDown();
                    getCursorPositionForOffset( insertOffset, m_cursorRow, m_cursorCol );
                }
                draw( false, false );
            }
            m_terminal.moveCursor( "Editor", m_cursorRow, m_cursorCol );
        }
        // TODO: We could do a shift-tab here to delete up to four spaces -
        // we'd need to work out how many spaces were behind the cursor in
        // the current line...
    }
    else if( c != '\r' )
    {
        if( m_openMode & modeWrite )
        {
            // FIXME: Filter for printable/legal characters?
            insertOffset = ( getOffsetAtCursor() + 1 );
            insertCharacter( c );
            if( !fastInsert( c ) )
            {
                // Need to re-draw screen.
                paginate();
                getCursorPositionForOffset( insertOffset, m_cursorRow, m_cursorCol );
                if( m_cursorRow == -1 )
                {
                    // Offset off bottom of screen.
                    scrollDown();
                    getCursorPositionForOffset( insertOffset, m_cursorRow, m_cursorCol );
                }
                draw( false, false );
            }
            m_terminal.moveCursor( "Editor", m_cursorRow, m_cursorCol );
        }
    }

    m_idleTimer->reset();
}

void Editor::run()
{
    if( ( m_needsRedraw || m_needsHighlight ) &&
        ( m_idleTimer->ms() >= idleTimeout ) )
    {
        bool doHighlight( m_highlighterFactory != nullptr );
        _draw( doHighlight, false );
        m_needsRedraw = false;
    }
}

void Editor::paginate()
{
    int currentIndex( 0 );
    int currentLogicalLine( 1 );
    int currentOffset( 0 );

    if( m_pageTopLine != m_lines.end() )
    {
        currentIndex = m_pageTopLine->m_index;
        currentLogicalLine = m_pageTopLine->m_logicalLine;
        currentOffset = m_pageTopLine->m_offset;
    }

    // Erase screen lines from top of screen to end.
    m_pageTopLine = m_lines.erase( m_pageTopLine, m_lines.end() );

    String lineStr;
    bool newline( false );
    bool error( false );
    bool eof( false );
    read( lineStr, currentOffset, newline, eof, error );
    while( !error && !eof ) // Read actual line.
    {
        // Split into words, on spaces.
        //std::cerr << "Paginate ll: " << currentLogicalLine << std::endl;
        Tokeniser tokeniser( lineStr, ' ' );
        String nextToken( tokeniser.token() );
        bool haveToken( true );
        while( haveToken )
        {
            // Create new logical (screen) line.
            Line line;
            line.m_offset = currentOffset;
            line.m_length = 0;
            line.m_index = currentIndex;
            line.m_logicalLine = currentLogicalLine;

            // Allocate words to logical (screen) line.
            //std::cerr << "Paginate tl offset: " << currentOffset << std::endl;
            int charsFree( m_terminal.width() );
            while( haveToken && ( charsFree - 2 ) > 0 && nextToken.length() <= ( charsFree - 2 ) ) // 2, to accommodate DOS EOL?
            {
                if( nextToken != "" )
                {
                    line.m_length += nextToken.length();
                    charsFree -= nextToken.length();
                    //std::cerr << "Add token " << nextToken << " ll: " << line.m_length << " cf: " << charsFree << std::endl;
                }

                if( !tokeniser.atEnd() )
                {
                    //std::cerr << "Get next token" << std::endl;
                    --charsFree;
                    ++line.m_length;
                    nextToken = tokeniser.token();
                    //std::cerr << "ht: " << haveToken << " cf: " << charsFree << " ntl: " << nextToken.length() << std::endl;
                }
                else
                {
                    //std::cerr << "Final token for line" << std::endl;
                    haveToken = false;
                }
            }

            // Token too large for single logical line. Force break between this
            // line and next, if any space left on this line, else defer
            // handling to next line (it will be broken there).
            if( ( nextToken.length() > ( m_terminal.width() - 2 ) ) &&
                ( ( charsFree - 2 ) > 0 ) )
            {
                line.m_length = ( m_terminal.width() - 2 );
                nextToken = nextToken.substr( charsFree - 2 );
            }

            // Account for newline bytes.
            if( !haveToken )
            {
                if( m_unixEOL )
                {
                    ++line.m_length;
                }
                else
                {
                    // FIXME: This is currently broken - users will see
                    // and be able to edit CR separately from LF !?
                    line.m_length += 2;
                }
            }

            currentOffset += line.m_length;

            //std::cerr << "Push line len: " << line.m_length << std::endl;

            m_lines.push_back( line );

            ++currentIndex;

            if( m_pageTopLine == m_lines.end() )
            {
                // Point to first repaginated line.
                m_pageTopLine--;
            }
        }

        ++currentLogicalLine;

        read( lineStr, currentOffset, newline, eof, error );
    }

    if( newline )
    {
        // Last line ended with \n. Create additional, empty, display line
        // so cursor can move beyond the final \n.
        Line line;
        line.m_offset = currentOffset;
        line.m_length = 0;
        line.m_logicalLine = currentLogicalLine;
        m_lines.push_back( line );
    }

    if( error ) m_error = true; // Flag errors to caller.
}

void Editor::_draw( bool highlight, bool fillScreen )
{
    bool error( false ); // Set true if error reading underlying file.

    // Fill blue background
    if( fillScreen )
    {
        m_terminal.fillScreen( '\0', Terminal::attributes( defaultBackground, defaultForeground ) );
    }
    
    // FIXME: We parse the entire file, not just the displayed lines,
    // to do highlighting, but that seems unavoidable unless we
    // have a special parser that can handle fragments (which I steered
    // away from, as having one parser seemed DRYer and more maintainable).
    
    // Create highlight lines.
    List< Line >::iterator currentLine( m_lines.begin() );
    Highlighter* highlighter( nullptr );
    Vector< Highlighter::Token > highlights;
    Vector< String > parseErrors;
    Vector< String > runtimeErrors;
    if( highlight )
    {
        highlighter = m_highlighterFactory->makeHighlighter();
        while( !error && ( currentLine != m_lines.end() ) )
        {
            // Pass line contents to highlighter.
            String lineStr;
            read( lineStr, currentLine->m_offset, currentLine->m_length, error );
            highlighter->line( lineStr, currentLine->m_logicalLine );

            ++currentLine;
        }

        // Retrieve all highlights created.
        highlighter->highlight( m_instanceName, m_modified, highlights, parseErrors, runtimeErrors );
    }

    // Draw all lines for the current screen. If highlights are enabled,
    // draw the highlights instead (it is assumed that the highlighter will
    // provide a colour for everything that should be on the screen).

    int currentRow( 0 ); // Relative to top of screen.
    currentLine = m_pageTopLine; // Iterator pointing to current logical line.
    int currentIndex( m_pageTopLine->m_index ); // Index of current logical line.
    Vector< Highlighter::Token >::const_iterator currentHighlight( highlights.begin() );
    while( !error &&
           ( currentRow < ( m_terminal.height() - 1 ) ) &&
           ( currentLine != m_lines.end() ) )
    {
        String lineStr;
        read( lineStr, currentLine->m_offset, currentLine->m_length, error );

        if( highlight )
        {
            while( ( currentHighlight != highlights.end() ) &&
                   ( currentHighlight->m_line < currentIndex ) )
            {
                // Try to cue up correct highlights for this logical line.
                ++currentHighlight;
            }

            while( ( currentHighlight != highlights.end() ) &&
                   ( currentHighlight->m_line == currentIndex ) )
            {
                // Draw highlights.
                int attributes( currentHighlight->m_attributes );
                if( !( attributes & 0xF0 ) )
                {
                    // Set blue background if background not already set.
                    attributes += ( defaultBackground << 4 );
                }

                if( currentHighlight->m_column < lineStr.length() )
                {
                    m_terminal.consumeNext( currentRow,
                                            currentHighlight->m_column,
                                            attributes );
                    m_terminal.consumeString( lineStr.substr( currentHighlight->m_column,
                                                            currentHighlight->m_length ),
                                            Terminal::scrollUnlock,
                                            Terminal::preserveBackground );
                }

                ++currentHighlight;
            }
        }
        else
        {
            // Else highlighting not enabled - draw entire logical line
            // in default colour.
            m_terminal.consumeNext( currentRow, 0 );
            m_terminal.consumeString( lineStr, true, Terminal::preserveBackground );
        }

        ++currentRow;
        ++currentLine;
        ++currentIndex;
    }

    // Draw caller-provided identifier text.
    m_terminal.consumeNext( m_terminal.height() - 1, 0, 0x9F );
    m_terminal.consumeString( m_displayName );

    m_terminal.redrawCursors();

    // Draw parse and runtime errors.
    if( m_errorsTerminal )
    {
        m_errorsTerminal->clearScreen();

        if( highlight )
        {
            {
            m_errorsTerminal->setAttributes( Terminal::attributes( Terminal::colYellow, Terminal::colBlack ) );
            Vector< String >::const_iterator it( parseErrors.begin() );
            for( ; it != parseErrors.end(); ++it )
            {
                m_errorsTerminal->consumeString( *it + "\r\n", Terminal::scrollLock );
            }
            }

            {
            m_errorsTerminal->setAttributes( Terminal::attributes( Terminal::colRed, Terminal::colWhite ) );
            Vector< String >::const_iterator it( runtimeErrors.begin() );
            for( ; it != runtimeErrors.end(); ++it )
            {
                m_errorsTerminal->consumeString( *it + "\r\n", Terminal::scrollLock );
            }
            }
        }
    }

    // Draw editor internal debugging output, if enabled.
    debugDrawLines();
    debugDrawChunks();

    // Show allowable keyboard commands.
    m_terminal.consumeNext( m_terminal.height() - 1, m_terminal.width() - m_terminal.countPrinting( m_keyGuide ) );
    m_terminal.consumeString( m_keyGuide, Terminal::scrollLock, Terminal::preserveBackground );

    if( highlight )
    {
        delete( highlighter );
    }

    m_needsHighlight = !highlight;

    if( error ) m_error = true; // Flag file read errors to caller.
}

int Editor::read( String& line, int from, bool& newline, bool& eof, bool& error )
{
    return _read( line, from, 0, newline, eof, readModeNewline, error );
}

int Editor::read( String& line, int from, int len, bool& error )
{
    bool newline( false );
    bool eof( false );
    return _read( line, from, len, newline, eof, readModeLen, error );
}

int Editor::_read( String& line, int from, int len, bool& newline, bool& eof, enum ReadMode readMode, bool& error )
{
    // Read data from chunks at arbitrary postion. Two modes are supported - in
    // length mode (used to draw screen rows and to save the edited file),
    // read from the given offset for the given length using any combination
    // of file and insert chunks. In newline mode (used in pagination to read
    // actual lines to split to logical lines), read from the given offset up
    // to, but not including, a \n character OR EOF and set "newline" bool
    // if a newline was encountered (unset for EOF).

    /*
    {
    LiteStream stream;
    stream << "Editor read: from " << from << " len " << len;
    LOG_DEBUG( stream.str() );
    }
    */

    error = false;

    line.clear();

    // Find chunk containing "from" offset.
    List< Chunk >::const_iterator it( m_chunks.begin() );
    int chunksOffset( 0 );
    for( ; ( it != m_chunks.end() ) && ( from >= ( chunksOffset + it->m_len ) ); ++it )
    {
        chunksOffset += it->m_len;
    }

    if( ( it == m_chunks.end() ) && ( from >= chunksOffset ) )
    {
        // EOF.
        //LOG_DEBUG( "Editor read: EOF" );
        eof = true;
        return 0;
    }

    char c( '\0' );
    int chunkRemain( 0 );
    int lenRead( 0 );
    while( !error &&
           ( it != m_chunks.end() ) &&
           ( ( readMode == readModeLen && ( lenRead < len ) ) || // Note mode-based termination condition.
             ( readMode == readModeNewline && ( c != '\n' ) ) ) )
    {
        chunkRemain = ( chunksOffset + it->m_len ) - from;

        /*
        {
        LiteStream stream;
        stream << "Editor read: chunksOffset " << chunksOffset << " chunkRemain " << chunkRemain;
        LOG_DEBUG( stream.str() );
        }
        */

        if( it->m_type == Chunk::file )
        {
            // Read from file one whole buffer at a time, but then process
            // one character at a time (required to look for newlines).
            char buffer[bufferSize];
            int fileOffset( it->m_offset + ( from - chunksOffset ) );
            int fileToRead( ( chunkRemain <= bufferSize ) ? chunkRemain : bufferSize );
            int fileDidRead( m_assetLoader->read( buffer, fileOffset, fileToRead ) );
            error = m_assetLoader->error();
            
            /*
            {
            LiteStream stream;
            stream << "Editor read: File chunk fileOffset " << fileOffset << " fileToRead " << fileToRead << " fileDidRead " << fileDidRead;
            LOG_DEBUG( stream.str() );
            }
            */

            int bufferPos( 0 );
            if( fileDidRead ) c = buffer[bufferPos++]; // Prepare next character
            // FIXME: Handle DOS line endings? (Strip both CR and LF.)
            while( !error &&
                   ( fileDidRead > 0 ) &&
                   ( chunkRemain > 0 ) &&
                   ( ( readMode == readModeLen && ( lenRead < len ) ) ||
                     ( readMode == readModeNewline && ( c != '\n' ) ) ) )
            {
                line += c;
                ++fileOffset;
                --chunkRemain;
                ++lenRead;

                /*
                {
                LiteStream stream;
                stream << "Editor read: File chunk fileOffset " << fileOffset << " chunkRemain " << chunkRemain << " lenRead " << lenRead;
                LOG_DEBUG( stream.str() );
                }
                */

                if( ( bufferPos == fileDidRead ) && ( chunkRemain > 0 ) )
                {
                    fileToRead = ( chunkRemain <= bufferSize ) ? chunkRemain : bufferSize;
                    fileDidRead = m_assetLoader->read( buffer, fileOffset, fileToRead );
                    error = m_assetLoader->error();
                    bufferPos = 0;
                }

                /*
                {
                LiteStream stream;
                stream << "Editor read: File chunk fileOffset " << fileOffset << " fileToRead " << fileToRead << " fileDidRead " << fileDidRead;
                LOG_DEBUG( stream.str() );
                }
                */

                if( ( chunkRemain > 0 ) &&
                    ( bufferPos < fileDidRead ) )
                {
                    c = buffer[bufferPos++]; // Prepare next character
                }
            }
        }
        else if( it->m_type == Chunk::inserted )
        {
            int insertedOffset( from - chunksOffset );

            /*
            {
            LiteStream stream;
            stream << "Editor read: Inserted chunk insertedOffset " << insertedOffset;
            LOG_DEBUG( stream.str() );
            }
            */

            c = it->m_text[ insertedOffset ]; // Prepare next character
            // FIXME: Handle DOS line endings? (Strip both CR and LF.)
            while( ( chunkRemain > 0 ) &&
                   ( ( readMode == readModeLen && ( lenRead < len ) ) ||
                     ( readMode == readModeNewline && ( c != '\n' ) ) ) )
            {
                line += c;
                ++insertedOffset;
                --chunkRemain;
                ++lenRead;

                /*
                {
                LiteStream stream;
                stream << "Editor read: Inserted chunk insertedOffset " << insertedOffset << " chunkRemain " << chunkRemain << " lenRead " << lenRead;
                LOG_DEBUG( stream.str() );
                }
                */

                if( chunkRemain > 0 )
                {
                    c = it->m_text[ insertedOffset ];
                }
            }
        }

        // FIXME: We assume the previous chunk read OK and was exhausted, but
        // that's not the case if there was a file chunk read error and
        // read() on the file returned zero previously! How to indicate
        // failure to the caller here, and can the caller handle failure?
        chunksOffset += it->m_len;
        from = chunksOffset;
        ++it;
    }
    
    newline = ( c == '\n' );
    eof = false;

    /*
    {
    LiteStream stream;
    stream << "Editor read: lenRead " << lenRead;
    LOG_DEBUG( stream.str() );
    }
    */

    return lenRead;
}

void Editor::debugDrawLines()
{
    if( m_debugChunksTerminal )
    {
        m_debugLinesTerminal->fillScreen( '\0', 0x3F );
        m_debugLinesTerminal->consumeNext( 0, 0, 0x3F );

        List< Line >::iterator it( m_pageTopLine );
        for( ; it != m_lines.end(); ++it )
        {
            LiteStream stream;
            stream << it->m_logicalLine << " " << it->m_length << " " << it->m_offset;
            String out( stream.str() );
            if( out.length() > m_debugLinesTerminal->width() ) out.erase( m_debugLinesTerminal->width() );
            if( out.length() < m_debugLinesTerminal->width() ) out += "\r\n";
            m_debugLinesTerminal->consumeString( out, true, Terminal::preserveBackground );
        }
    }
}

void Editor::debugDrawChunks()
{
    if( m_debugChunksTerminal )
    {
        m_debugChunksTerminal->fillScreen( '\0', 0x5F );
        m_debugChunksTerminal->consumeNext( 0, 0, 0x5F );

        {
            LiteStream stream;
            stream << m_cursorRow << "," << m_cursorCol << ":" << getOffsetAtCursor() << " ";
            m_debugChunksTerminal->consumeString( stream.str(), true, Terminal::preserveBackground );
        }

        List< Chunk >::const_iterator it( m_chunks.begin() );
        for( ; it != m_chunks.end(); ++it )
        {
            LiteStream stream;
            if( it->m_type == Chunk::file )
            {
                m_debugChunksTerminal->consumeChar( 'F', true, Terminal::preserveBackground );
            }
            else if( it->m_type == Chunk::inserted )
            {
                m_debugChunksTerminal->consumeChar( 'I', true, Terminal::preserveBackground );
            }

            stream << it->m_offset << "," << it->m_len << " ";
            m_debugChunksTerminal->consumeString( stream.str(), true, Terminal::preserveBackground );
        }
    }
}

bool Editor::tryMoveCursor( int row, int col )
{
    bool couldMove( false );

    if( ( col >= 0 ) && ( col < m_terminal.width() ) &&
        ( row >= 0 ) && ( row < ( m_terminal.height() - 1 ) ) )
    {
        List< Line >::iterator lineIt( getLineByRow( row ) );
        if( lineIt != m_lines.end() )
        {
            const Line& line( *lineIt );
            m_cursorRow = row;
            if( col >= line.m_length )
            {
                if( line.m_length > 0 )
                {
                    m_cursorCol = line.m_length - 1;
                }
                else
                {
                    // Special case of at end of text - zero line length.
                    m_cursorCol = 0;
                }
            }
            else
            {
                m_cursorCol = col;
                couldMove = true;
            }

            m_terminal.moveCursor( "Editor", m_cursorRow, m_cursorCol );
        }
    }

    return couldMove;
}

bool Editor::scrollUp()
{
    if( m_pageTopLine != m_lines.begin() )
    {
        --m_pageTopLine;
        return true;
    }

    return false;
}

bool Editor::scrollDown()
{
    List< Line >::const_iterator lineIt( m_pageTopLine );
    int terminalLine( 0 );
    // Return if not at least one more line 
    for( ; terminalLine < m_terminal.height(); ++terminalLine )
    {
        if( lineIt == m_lines.end() )
        {
            return false;
        }
        ++lineIt;
    }

    ++m_pageTopLine;

    return true;
}

void Editor::insertCharacter( char c )
{
    //std::cout << "Insert char" << std::endl;
    List< Chunk >::iterator insertChunkIter( m_chunks.end() );
    int insertChunkOffset( 0 );

    // Find existing insert block, or split file block.
    int cursorOffset( getOffsetAtCursor() );
    //std::cout << "Cursor at offset " << cursorOffset << std::endl;
    int chunksOffset( 0 );
    List< Chunk >::iterator it( m_chunks.begin() );
    for( ; it != m_chunks.end(); ++it )
    {
        //std::cout << chunksOffset << "," << it->m_offset << "," << it->m_len << std::endl;
        if( ( it->m_type == Chunk::inserted ) &&
            ( cursorOffset <= ( chunksOffset + it->m_len ) ) )
        {
            //std::cout << "Use existing insert chunk" << std::endl;
            insertChunkIter = it;
            insertChunkOffset = cursorOffset - chunksOffset;
            break;
        }
        else if( cursorOffset == chunksOffset )
        {
            //std::cout << "Create insert chunk before" << std::endl;
            // Insert chunk before,
            Chunk insertChunk;
            insertChunk.m_type = Chunk::inserted;
            insertChunk.m_offset = -1;
            insertChunk.m_len = 0;

            insertChunkIter = m_chunks.insert( it, insertChunk );
            break;
        }
        else if( cursorOffset < ( chunksOffset + it->m_len ) )
        {
            //std::cout << "Splice insert chunk" << std::endl;

            // Truncate current chunk.
            int origOffset( it->m_offset );
            int origLen( it->m_len );
            int spliceOffset( cursorOffset - chunksOffset );
            it->m_len = spliceOffset;

            // Insert new chunk after current.
            Chunk insertChunk;
            insertChunk.m_type = Chunk::inserted;
            insertChunk.m_offset = -1;
            insertChunk.m_len = 0;

            it = m_chunks.insert( ++it, insertChunk );

            // Insert new file chunk for remainder of split.
            Chunk fileChunk;
            fileChunk.m_type = Chunk::file;
            fileChunk.m_offset = origOffset + spliceOffset;
            fileChunk.m_len = origLen - spliceOffset;

            insertChunkIter = m_chunks.insert( ++it, fileChunk );

            // Insert chunk is previous.
            --insertChunkIter;
            break;
        }

        chunksOffset += it->m_len;
    }

    if( insertChunkIter == m_chunks.end() )
    {
        // At end after file chunk. Create new insert chunk.
        Chunk insertChunk;
        insertChunk.m_type = Chunk::inserted;
        insertChunk.m_offset = -1;
        insertChunk.m_len = 0;

        insertChunkIter = m_chunks.insert( it, insertChunk );
    }

    // FIXME: Sanity check offset.
    insertChunkIter->m_text.insert( insertChunkOffset, 1, c );
    ++insertChunkIter->m_len;

    m_modified = true;
}

void Editor::deleteCharacter( bool previous )
{
    int deleteOffset( getOffsetAtCursor() );
    if( previous )
    {
        --deleteOffset;
    }

    if( deleteOffset >= 0 )
    {
        // Find chunk for delete offset.
        List< Chunk >::iterator it( m_chunks.begin() );
        int chunksOffset( 0 );
        for( ; ( it != m_chunks.end() ) && ( deleteOffset >= ( chunksOffset + it->m_len ) ); ++it )
        {
            chunksOffset += it->m_len;
        }

        if( it != m_chunks.end() )
        {
            if( it->m_type == Chunk::file )
            {
                // Splice.
                if( deleteOffset == chunksOffset )
                {
                    ++it->m_offset;
                    --it->m_len;
                }
                else if( deleteOffset == ( chunksOffset + it->m_len - 1 ) )
                {
                    --it->m_len;
                }
                else
                {
                    //std::cout << "Splice delete chunk" << std::endl;

                    // Truncate current chunk.
                    int origOffset( it->m_offset );
                    int origLen( it->m_len );
                    int spliceOffset( deleteOffset - chunksOffset );
                    it->m_len = spliceOffset;

                    // Insert new file chunk for remainder of split.
                    Chunk fileChunk;
                    fileChunk.m_type = Chunk::file;
                    fileChunk.m_offset = origOffset + spliceOffset + 1;
                    fileChunk.m_len = origLen - spliceOffset - 1;

                    m_chunks.insert( ++it, fileChunk );
                }
            }
            else if( it->m_type == Chunk::inserted )
            {
                // Erase from inserted characters.
                it->m_text.erase( ( deleteOffset - chunksOffset ), 1 );
                --it->m_len;
            }

            if( it->m_len == 0 )
            {
                // Kill empty chunk.
                m_chunks.erase( it );
            }
        }

        m_modified = true;
    }
}

bool Editor::fastInsert( char c )
{
    if( ( m_cursorCol < ( m_terminal.width() - 1 ) ) && ( c != '\n' ) )
    {
        List< Line >::iterator lineIt( getLineByRow( m_cursorRow ) );
        if( lineIt->m_length < m_terminal.width() )
        {
            m_terminal.insertAtCursor( c, "Editor" );
            
            if( lineIt != m_lines.end() )
            {
                ++(lineIt->m_length);

                while( ++lineIt != m_lines.end() )
                {
                    ++(lineIt->m_offset);
                }

                ++m_cursorCol; // Caller to move cursor on terminal.

                m_needsHighlight = true;

                debugDrawLines();
                debugDrawChunks();

                return true;
            }
        }
    }

    return false;
}

bool Editor::fastBackspace()
{
    if( m_cursorCol > 0 )
    {
        m_terminal.backspaceAtCursor( "Editor" );
        List< Line >::iterator lineIt( getLineByRow( m_cursorRow ) );
        if( lineIt != m_lines.end() )
        {
            --(lineIt->m_length);

            while( ++lineIt != m_lines.end() )
            {
                --(lineIt->m_offset);
            }

            --m_cursorCol; // Caller to move cursor on terminal.

            m_needsHighlight = true;

            debugDrawLines();
            debugDrawChunks();

            return true;
        }
    }

    return false;
}

List< Editor::Line >::iterator Editor::getLineByRow( int row )
{
    // Get logical line for the given terminal row.
    if( row < m_lines.size() )
    {
        // FIXME: Pity we lose operator[] as opposed to Vector,
        // but we gain iterator validity after insert, which is
        // necessary for draw().
        List< Line >::iterator it( m_pageTopLine );
        for( int i = 0; i < row && it != m_lines.end(); ++i )
        {
            ++it;
        }

        if( it != m_lines.end() )
        {
            return it;
        }
    }

    return m_lines.end();
}

int Editor::getOffsetAtCursor()
{
    // Get within-chunks offset for current cursor position.
    List< Line >::const_iterator currentLine( m_pageTopLine );

    for( int row = 0; row < m_cursorRow; ++row )
    {
        ++currentLine;
    }

    return( currentLine->m_offset + m_cursorCol );
}

void Editor::getCursorPositionForOffset( int offset, int& row, int& col )
{
    // Get cursor position for given within-chunks offset.
    row = 0;
    col = 0;

    if( offset < m_pageTopLine->m_offset )
    {
        // Offset before top of screen.
        row = -1;
        col = -1;
        return;
    }

    List< Line >::const_iterator it( m_pageTopLine );
    for( ; ( it != m_lines.end() ) && ( it->m_length > 0 ); ++it )
    {
        if( ( offset - it->m_offset ) < it->m_length )
        {
            col = ( offset - it->m_offset );
            break;
        }

        ++row;

        if( row == ( m_terminal.height() - 1 ) )
        {
            // Offset after bottom of screen.
            row = -1;
            col = -1;
            break;
        }
    }

    //std::cout << "Cursor position for offset " << offset << " is " << row << "," << col << std::endl;
}

} // namespace Editor

} // namespace Agape
