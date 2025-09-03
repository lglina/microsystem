#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Utils/Tokeniser.h"
#include "World/WorldCoordinates.h"
#include "Form.h"
#include "FormTypes.h"
#include "Collections.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

using namespace Agape::InputDevices;

namespace Agape
{

namespace UI
{

namespace Forms
{

Form::Form( WindowManager& windowManager,
            const String& windowName,
            const String& uiAssetName,
            const Vector< struct Field >& fields,
            struct Title title ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_uiAssetName( uiAssetName ),
  m_fields( fields ),
  m_currentFieldIter( m_fields.end() ),
  m_title( title ),
  m_uiAssetLoader( nullptr ),
  m_uiOpen( false ),
  m_interactive( false )
{
    Vector< struct Field >::iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        if( it->m_enabled )
        {
            // There is at least one selectable field.
            m_currentFieldIter = it;
            m_interactive = true;
            break;
        }
    }
}

Form::~Form()
{
    close();

    if( m_uiAssetLoader )
    {
        delete( m_uiAssetLoader );
    }
}

bool Form::openUI()
{
    // FIXME: Should the caller be responsible for making the window
    // visible, or should we do that? If we're only using WindowManager
    // to get the TerminalWindow/Terminal, should that just be passed
    // in by the caller to remove the dependency on WindowManager?
    if( m_windowManager.getTerminalWindow( m_windowName, m_terminalWindow ) )
    {
        // It is assumed all UI will be baked.
        if( !m_uiAssetName.empty() && !m_uiAssetLoader )
        {
            m_uiAssetLoader = new AssetLoaders::Baked( World::Coordinates(), m_uiAssetName );
            if( m_uiAssetLoader->open() )
            {
                m_uiOpen = true;
            }
        }
        else if( m_uiAssetName.empty() )
        {
            // No asset specified.
            m_uiOpen = true;
        }
    }

    return m_uiOpen;
}

bool Form::uiIsOpen() const
{
    return m_uiOpen;
}

void Form::setTitle( const Title& title )
{
    m_title = title;
}

void Form::reset()
{
    m_currentFieldIter = m_fields.end();
    Vector< struct Field >::iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        if( it->m_type == fText )
        {
            it->m_contents.clear();
        }
        else if( it->m_type == fSelect )
        {
            SelectXProps* xprops( static_cast< SelectXProps* >( it->m_xprops ) );
            xprops->m_selection = 0;
            xprops->m_topValue = -1;
        }

        if( m_currentFieldIter == m_fields.end() &&
            it->m_enabled )
        {
            m_currentFieldIter = it;
        }
    }
}

void Form::draw()
{
    if( !m_uiOpen )
    {
        return;
    }

    m_terminalWindow.m_terminal->consumeNext( 0, 0 );

    if( !m_uiAssetName.empty() )
    {
        Assets::ANSIFile uiAsset( *m_uiAssetLoader );
        m_terminalWindow.m_terminal->consumeAsset( uiAsset, uiAsset.dataSize(), true );
    }

    drawTitle();

    // Pre-fill form.
    Vector< struct Field >::iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        drawLabel( it );

        if( it->m_type != fButton )
        {
            clearField( it );

            if( !it->m_contents.empty() )
            {
                if( it->m_type == fText )
                {
                    drawText( it );
                }
                else if( it->m_type == fSelect )
                {
                    drawSelect( it, ( m_currentFieldIter == it ) );
                }
            }
        }
    }

    // Enable cursor.
    if( m_currentFieldIter != m_fields.end() && m_currentFieldIter->m_enabled  )
    {
        if( m_currentFieldIter->m_type == fText )
        {
            positionTextCursor();
            m_terminalWindow.m_terminal->enableTerminalCursor( true );
        }
        else if( m_currentFieldIter-> m_type == fButton )
        {
            drawButtonActive( m_currentFieldIter );
        }
    }
}
 
void Form::close()
{
    m_terminalWindow.m_terminal->enableTerminalCursor( false );
}

void Form::consumeChar( char c )
{
    if( !m_uiOpen )
    {
        return;
    }

    if( !m_interactive )
    {
        return;
    }

    if( c == Key::tab ||
        ( m_currentFieldIter->m_type == fButton &&
          ( c == Key::down || c == Key::right ) ) )
    {
        nextField();
    }
    else if( c == Key::shiftTab ||
             ( m_currentFieldIter->m_type == fButton &&
               ( c == Key::up || c == Key::left ) ) )
    {
        prevField();
    }
    else if( m_currentFieldIter->m_type == fSelect &&
             c == Key::up )
    {
        prevSelect();
    }
    else if( m_currentFieldIter->m_type == fSelect &&
             c == Key::down )
    {
        nextSelect();
    }
    else if( m_currentFieldIter->m_type == fText &&
             ( ( c == Key::left ) || ( c == Key::right ) ||
               ( c == Key::up ) || ( c == Key::down ) ) )
    {
        textMoveCursor( c );
    }
    else if( m_currentFieldIter->m_type == fText &&
             c == Key::backspace )
    {
        textBackspace();
    }
    else if( m_currentFieldIter->m_type == fText &&
             ( ( c >= 'a' && c <= 'z' ) ||
               ( c >= 'A' && c <= 'Z' ) ||
               ( c >= '0' && c <= '9' ) ||
               ( ( m_currentFieldIter->m_permittedSymbols == "" ) ||
                 ( m_currentFieldIter->m_permittedSymbols.find( c ) != String::npos ) ) ) )
    {
        textAdd( c );
    }
}

void Form::setFieldContents( const String& fieldName, const String& contents )
{
    if( !m_uiOpen )
    {
        return;
    }

    Vector< struct Field >::iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        if( it->m_name == fieldName )
        {
            it->m_contents = contents;
            break;
        }
    }
}

String Form::getFieldContents( const String& fieldName ) const
{
    if( !m_uiOpen )
    {
        return String();
    }

    Vector< struct Field >::const_iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        if( it->m_name == fieldName )
        {
            if( it->m_type == fText )
            {
                return it->m_contents;
            }
            else if( it->m_type == fSelect )
            {
                const SelectXProps* xprops( static_cast< const SelectXProps* >( it->m_xprops ) );
                int entryNum( 0 );
                Tokeniser tokeniser( it->m_contents, xprops->m_delim );
                String token( tokeniser.token() );
                while( !token.empty() && entryNum != xprops->m_selection )
                {
                    ++entryNum;
                    token = tokeniser.token();
                }
                return token;
            }
        }
    }

    return String();
}

int Form::getSelectIndex( const String& fieldName ) const
{
    if( !m_uiOpen )
    {
        return -1;
    }

    Vector< struct Field >::const_iterator it;
    for( it = m_fields.begin(); it != m_fields.end(); ++it )
    {
        if( ( it->m_name == fieldName ) && ( it->m_type == fSelect ) )
        {
            const SelectXProps* xprops( static_cast< const SelectXProps* >( it->m_xprops ) );
            if( ( xprops->m_selection >= 0 ) && ( xprops->m_selection < xprops->m_numValues ) )
            {
                return xprops->m_selection;
            }
        }
    }

    return -1;
}

void Form::setSelectIndex( const String& fieldName, int index )
{
    if( m_uiOpen )
    {
        Vector< struct Field >::const_iterator it;
        for( it = m_fields.begin(); it != m_fields.end(); ++it )
        {
            if( ( it->m_name == fieldName ) && ( it->m_type == fSelect ) )
            {
                SelectXProps* xprops( static_cast< SelectXProps* >( it->m_xprops ) );
                if( index < xprops->m_numValues )
                {
                    xprops->m_selection = index;
                    clearField( m_currentFieldIter );
                    drawSelect( m_currentFieldIter, ( m_currentFieldIter == it ) );
                }
            }
        }
    }
}

const Field& Form::currentField() const
{
    return *m_currentFieldIter;
}

bool Form::setCurrentField( const String& fieldName )
{
    Vector< struct Field >::iterator fieldIter( m_fields.begin() );
    for( ; fieldIter != m_fields.end(); ++fieldIter )
    {
        if( fieldIter->m_name == fieldName )
        {
            m_currentFieldIter = fieldIter;
            return true;
        }
    }

    return false;
}

void Form::nextField()
{
    Vector< struct Field >::iterator nextFieldIter( m_currentFieldIter );
    do
    {
        ++nextFieldIter;
        if( nextFieldIter == m_fields.end() )
        {
            nextFieldIter = m_fields.begin();
        }
    } while( !nextFieldIter->m_enabled );

    switchField( m_currentFieldIter, nextFieldIter );
}

void Form::prevField()
{
    Vector< struct Field >::iterator prevFieldIter( m_currentFieldIter );
    do
    {
        if( prevFieldIter != m_fields.begin() )
        {
            --prevFieldIter;
        }
        else
        {
            Vector< struct Field >::iterator lastField( m_fields.end() );
            --lastField;
            prevFieldIter = lastField;
        }
    } while( !prevFieldIter->m_enabled );

    switchField( m_currentFieldIter, prevFieldIter );
}

void Form::nextSelect()
{
    if( m_currentFieldIter->m_type == fSelect )
    {
        SelectXProps* xprops( static_cast< SelectXProps* >( m_currentFieldIter->m_xprops ) );
        if( xprops->m_selection < xprops->m_numValues - 1 )
        {
            ++(xprops->m_selection);
            clearField( m_currentFieldIter );
            drawSelect( m_currentFieldIter, true ); // true = focused.
        }
    }
}

void Form::prevSelect()
{
    if( m_currentFieldIter->m_type == fSelect )
    {
        SelectXProps* xprops( static_cast< SelectXProps* >( m_currentFieldIter->m_xprops ) );
        if( xprops->m_selection > 0 )
        {
            --(xprops->m_selection);
            clearField( m_currentFieldIter );
            drawSelect( m_currentFieldIter, true ); // true = focused.
        }
    }
}

void Form::clearField( Vector< struct Field >::iterator& field )
{
    for( int row = 0; row < field->m_height; ++row )
    {
        m_terminalWindow.m_terminal->consumeNext( field->m_row + row, field->m_col, field->m_fieldAttributes );
        for( int col = 0; col < field->m_width; ++col )
        {
            m_terminalWindow.m_terminal->consumeChar( ' ', Terminal::scrollLock );
        }
    }
}

void Form::drawTitle()
{
    int col( m_title.m_col );
    if( m_title.m_centre )
    {
        col = ( m_title.m_col + ( m_title.m_width / 2 ) ) - ( m_title.m_title.length() / 2 );
        if( col < 0 ) col = 0;
    }

    m_terminalWindow.m_terminal->consumeNext( m_title.m_row, col, m_title.m_attributes );
    m_terminalWindow.m_terminal->consumeString( m_title.m_title, false, Terminal::preserveBackground );
}

void Form::drawLabel( Vector< struct Field >::iterator& field )
{
    if( field->m_labelRow != -1 && field->m_labelCol != -1 )
    {
        m_terminalWindow.m_terminal->consumeNext( field->m_labelRow, field->m_labelCol, field->m_labelAttributes );
        m_terminalWindow.m_terminal->consumeString( field->m_name, false, Terminal::preserveBackground );
    }
}

void Form::drawText( Vector< struct Field >::iterator& field )
{
    int contentsSize( field->m_contents.size() );
    int charsDrawn( 0 );
    int currentLine( 0 );
    while( ( charsDrawn < contentsSize ) && ( currentLine < field->m_height ) )
    {
        m_terminalWindow.m_terminal->consumeNext( field->m_row + currentLine, field->m_col, field->m_fieldAttributes );

        int charsToDraw( contentsSize - charsDrawn );
        charsToDraw = ( charsToDraw > field->m_width ) ? field->m_width : charsToDraw;
        m_terminalWindow.m_terminal->consumeString( field->m_contents.substr( charsDrawn, charsToDraw ) );
        charsDrawn += charsToDraw;
        ++currentLine;
    }
}

void Form::drawSelect( Vector< struct Field >::iterator& field, bool focused )
{
    SelectXProps* xprops( static_cast< SelectXProps* >( field->m_xprops ) );
    if( xprops->m_topValue == -1 )
    {
        xprops->m_topValue = xprops->m_selection;
    }
    else if( xprops->m_selection == xprops->m_topValue + field->m_height )
    {
        ++( xprops->m_topValue );
    }
    else if( xprops->m_selection < ( xprops->m_topValue ) )
    {
        --( xprops->m_topValue );
    }

    int entryNum( 0 );
    Tokeniser tokeniser( field->m_contents, xprops->m_delim );
    String token( tokeniser.token() );
    while( token != "" )
    {
        int entryOffset( entryNum - xprops->m_topValue );
        if( entryOffset >= 0 && entryOffset < field->m_height )
        {
            int attributes;
            
            if( entryNum == xprops->m_selection )
            {
                if( focused )
                {
                    attributes = xprops->m_selectionAttributes;
                }
                else
                {
                    attributes = xprops->m_selectionNotFocusedAttributes;
                }
            }
            else
            {
                attributes = field->m_fieldAttributes;
            }

            int printingWidth( m_terminalWindow.m_terminal->countPrinting( token ) );
            if( printingWidth > field->m_width )
            {
                token = token.substr( 0, field->m_width - 3 );
                token += "...";
            }

            m_terminalWindow.m_terminal->consumeNext( field->m_row + entryOffset, field->m_col, attributes );
            m_terminalWindow.m_terminal->consumeString( token );
        }

        ++entryNum;

        token = tokeniser.token();
    }

    xprops->m_numValues = entryNum;
}

void Form::drawButtonActive( Vector< struct Field >::iterator& field )
{
    ButtonXProps* xprops( static_cast< ButtonXProps* >( field->m_xprops ) );
    m_terminalWindow.m_terminal->consumeNext( field->m_labelRow, field->m_labelCol, xprops->m_selectionAttributes );
    m_terminalWindow.m_terminal->consumeString( field->m_name );
}

void Form::positionTextCursor()
{
    // FIXME: Support multi-line fields.
    if( m_currentFieldIter->m_contents.size() < ( m_currentFieldIter->m_width * m_currentFieldIter->m_height ) )
    {
        // Position cursor after last character.
        int nextEntryRow( m_currentFieldIter->m_row + ( m_currentFieldIter->m_contents.size() / m_currentFieldIter->m_width ) );
        int nextEntryCol( m_currentFieldIter->m_col + ( m_currentFieldIter->m_contents.size() % m_currentFieldIter->m_width ) );
        m_terminalWindow.m_terminal->consumeNext( nextEntryRow, nextEntryCol );
    }
    else
    {
        // Position cursor on last character.
        int lastEntryCol( m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1 );
        m_terminalWindow.m_terminal->consumeNext( m_currentFieldIter->m_row + m_currentFieldIter->m_height - 1, lastEntryCol );
    }
}

void Form::switchField( Vector< struct Field >::iterator& current, const Vector< struct Field >::iterator& next )
{
    if( next != current )
    {
        if( current->m_type == fSelect )
        {
            drawSelect( current, false ); // false = not focused.
        }
        else if( current->m_type == fButton )
        {
            drawLabel( current ); // Draw inactive button.
        }

        current = next;

        if( current->m_type == fText )
        {
            positionTextCursor();
            m_terminalWindow.m_terminal->enableTerminalCursor( true );
        }
        else if( current->m_type == fSelect )
        {
            m_terminalWindow.m_terminal->enableTerminalCursor( false );
            drawSelect( current, true ); // true = focused.
        }
        else if( current->m_type == fButton )
        {
            m_terminalWindow.m_terminal->enableTerminalCursor( false );
            drawButtonActive( current );
        }
    }
}

int Form::textOffsetAtCursor()
{
    int row( 0 );
    int col( 0 );
    if( !m_terminalWindow.m_terminal->getCursor( "Terminal", row, col ) ) return 0;

    return( ( ( row - m_currentFieldIter->m_row ) * m_currentFieldIter->m_width ) + ( col - m_currentFieldIter->m_col ) );
}

void Form::textAdd( char c )
{
    // Don't add if already at max length.
    TextXProps* xprops( static_cast< TextXProps* >( m_currentFieldIter->m_xprops ) );
    if( m_currentFieldIter->m_contents.size() == xprops->m_maxLength )
    {
        return;
    }

    int row( 0 );
    int col( 0 );
    if( !m_terminalWindow.m_terminal->getCursor( "Terminal", row, col ) ) return;

    int numFullRows( m_currentFieldIter->m_contents.size() / m_currentFieldIter->m_width );

    // Insert into field contents.
    m_currentFieldIter->m_contents.insert( textOffsetAtCursor(), 1, c );

    bool needMoveCursor( false );
    if( row == ( m_currentFieldIter->m_row + numFullRows ) )
    {
        // If within last row of current contents AND row not full, do fast insert.
        // Don't need to move cursor as terminal does this for us, unless we go
        // off the end of the field (see below).
        m_terminalWindow.m_terminal->insertAtCursor( c, "Terminal", m_currentFieldIter->m_col, m_currentFieldIter->m_width );
    }
    else
    {
        // Else redraw whole text area and manually move cursor.
        drawText( m_currentFieldIter );
        needMoveCursor = true;
    }

    // Past edge of field?
    ++col;
    if( col == ( m_currentFieldIter->m_col + m_currentFieldIter->m_width ) )
    {
        // Can move to next row?
        if( ( row - m_currentFieldIter->m_row + 1 ) < m_currentFieldIter->m_height )
        {
            // Move to the next row.
            ++row;
            col = m_currentFieldIter->m_col;
        }
        else
        {
            // Position cursor on last character of current row.
            col = m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1;
        }
        needMoveCursor = true;
    }

    if( needMoveCursor )
    {
        m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
    }
}

void Form::textMoveCursor( char c )
{
    int row( 0 );
    int col( 0 );
    if( !m_terminalWindow.m_terminal->getCursor( "Terminal", row, col ) ) return;

    if( c == Key::left )
    {
        if( col > m_currentFieldIter->m_col )
        {
            // Not at start of row. Can move backward.
            --col;
        }
        else if( row > m_currentFieldIter->m_row )
        {
            // Already at start of row. Can move to the end of the previous row?
            --row;
            col = m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1;
        }
        m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
    }
    else if( c == Key::right )
    {
        if( col < m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1 )
        {
            // Not at end of row. Can move forward.
            ++col;
        }
        else if( row < m_currentFieldIter->m_row + m_currentFieldIter->m_height - 1 )
        {
            // Already at end of row. Can move to the start of the next row?
            ++row;
            col = m_currentFieldIter->m_col;
        }

        // Make sure we're not moving more than one space beyond
        // the end of the current field contents.
        int valueOffset( ( ( row - m_currentFieldIter->m_row ) * m_currentFieldIter->m_width ) + ( col - m_currentFieldIter->m_col ) );
        if( valueOffset <= m_currentFieldIter->m_contents.length() )
        {
            m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
        }
    }
    else if( c == Key::up )
    {
        if( row > m_currentFieldIter->m_row )
        {
            // Can move to previous row.
            --row;
            m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
        }
    }
    else if( c == Key::down )
    {
        int numFullRows( m_currentFieldIter->m_contents.size() / m_currentFieldIter->m_width );
        int lastRowNumChars( m_currentFieldIter->m_contents.size() % m_currentFieldIter->m_width );

        // Can move to next row in field and that row has contents?
        ++row;
        if( ( row < ( m_currentFieldIter->m_row + m_currentFieldIter->m_height ) ) &&
            ( row <= ( m_currentFieldIter->m_row + numFullRows ) ) )
        {
            if( ( row == ( m_currentFieldIter->m_row + numFullRows ) ) &&
                ( col > ( m_currentFieldIter->m_col + lastRowNumChars ) ) )
            {
                // If the cursor would land more than one space beyond the end
                // of the contents in the last row, move it to the left.
                col = ( m_currentFieldIter->m_col + lastRowNumChars );
            }
            m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
        }
    }
}

void Form::textBackspace()
{
    int row( 0 );
    int col( 0 );
    if( !m_terminalWindow.m_terminal->getCursor( "Terminal", row, col ) ) return;

    // Don't backspace if already at start of field.
    if( ( row == m_currentFieldIter->m_row ) && ( col == m_currentFieldIter->m_col ) )
    {
        return;
    }

    int numFullRows( m_currentFieldIter->m_contents.size() / m_currentFieldIter->m_width );
    int lastRowNumChars( m_currentFieldIter->m_contents.size() % m_currentFieldIter->m_width );

    // Erase from field contents.
    m_currentFieldIter->m_contents.erase( textOffsetAtCursor() - 1, 1 );

    if( ( row == ( m_currentFieldIter->m_row + numFullRows ) ) &&
        ( col != m_currentFieldIter->m_col ) )
    {
        // If within last row AND row not empty, do fast backspace.
        // Don't need to move cursor as terminal does this for us.
        m_terminalWindow.m_terminal->backspaceAtCursor( "Terminal", m_currentFieldIter->m_col, m_currentFieldIter->m_width );
    }
    else
    {
        // Else redraw whole text area.
        drawText( m_currentFieldIter );

        // Need to erase what was previously the last character on the
        // last line.
        int eraseRow( m_currentFieldIter->m_row + numFullRows );
        int eraseCol( m_currentFieldIter->m_col + lastRowNumChars - 1 );
        if( lastRowNumChars == 0 )
        {
            // We backspaced into the previous row. Need to erase the last
            // character in that row.
            --eraseRow;
            eraseCol = m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1;
        }
        m_terminalWindow.m_terminal->consumeNext( eraseRow, eraseCol );
        m_terminalWindow.m_terminal->consumeChar( '\0' );

        // Past start of field? Move to end of previous row.
        --col;
        if( col < m_currentFieldIter->m_col )
        {
            --row;
            col = m_currentFieldIter->m_col + m_currentFieldIter->m_width - 1;
        }
        
        // Move cursor.
        m_terminalWindow.m_terminal->moveCursor( "Terminal", row, col );
    }
}

} // namespace Forms

} // namespace UI

} // namespace Agape
