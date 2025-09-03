#include "InputDevices/InputDevice.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "Utils/Tokeniser.h"
#include "World/WorldUtilities.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "PhonebookStrategy.h"
#include "Phonebook.h"
#include "StrategyHelper.h"
#include "StringConstants.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

namespace
{
    const int textWidth( 40 );
    const int textHeight( 1 );
    const int selectWidth( 50 );
    const int selectHeight( 10 );
    const int contentFirstRow( 8 );
    const int contentNoTitleFirstRow( 6 );
    const int contentCol( 8 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    const int labelAttributes( 0x07 );
    const int listLabelAttributes( 0 );

    const int titleRow( 6 );
    const int titleCol( 8 );
    const int titleWidth( 50 );
    const int titleAttributes( 0x0F );

    const int guideRow( 20 );

    const String textAssetName( "servers-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Phonebook::Phonebook( WindowManager& windowManager,
                      const String& windowName,
                      InputDevice& inputDevice,
                      Agape::Phonebook& phonebook,
                      Dialogue& dialogue ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_inputDevice( inputDevice ),
  m_phonebook( phonebook ),
  m_dialogue( dialogue ),
  m_state( select ),
  m_completed( false ),
  m_currentForm( nullptr ),
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Phonebook::~Phonebook()
{
    closeForm();
}

void Phonebook::enter( const Value& parameters )
{
    m_completed = false;

    drawSelectForm();
}

void Phonebook::returnTo( const Value& parameters )
{
}

bool Phonebook::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Phonebook::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        return true;
    }

    return false;
}

void Phonebook::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( m_state == select )
        {
            if( c == 'a' )
            {
                m_state = add;
                drawAddForm();
            }
            else if( c == 'x' )
            {
                deleteEntry();
                drawSelectForm();
            }
            else if( c == 'd' )
            {
                setDefaultEntry();
                drawSelectForm();
            }
            else if( c == '\x1b' )
            {
                m_completed = true;
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
        }
        else if( m_state == add )
        {
            if( c == '\n' )
            {
                addEntry();
                m_state = select;
                drawSelectForm();
            }
            else if( c == '\x1b' )
            {
                m_state = select;
                drawSelectForm();
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
        }
    }
}

void Phonebook::drawBackground()
{
    if( m_terminal != nullptr )
    {
        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void Phonebook::drawSelectForm()
{
    drawBackground();

    Map< String, String > entries( m_phonebook.getEntries() );
    String defaultName;
    String defaultNumber;
    m_phonebook.getDefaultEntry( defaultName, defaultNumber );
    String catEntryNames;
    Map< String, String >::const_iterator it( entries.begin() );
    for( ; it != entries.end(); ++it )
    {
        if( it != entries.begin() )
        {
            catEntryNames += ';';
        }

        catEntryNames += it->first + " \304 ";

        if( it->first == defaultName )
        {
            catEntryNames += "(default) \304 ";
        }

        catEntryNames += it->second;
    }

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _Name,
                                    selectWidth,
                                    selectHeight,
                                    contentNoTitleFirstRow,
                                    contentCol,
                                    fieldAttributes,
                                    catEntryNames,
                                    selectionAttributes,
                                    listLabelAttributes ) );

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mA\x1b[37m Add\x1b[0m  \x1b[97;100mX\x1b[37m Delete\x1b[0m  \x1b[97;100mD\x1b[37m Set default\x1b[0m  \x1b[97;100mEsc\x1b[37m Menu\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title() );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->reset();
        m_currentForm->draw();
    }
}

void Phonebook::drawAddForm()
{
    drawBackground();

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _Name,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow,
                                    contentCol,
                                    fieldAttributes,
                                    labelAttributes ) );

    fields.push_back( Forms::Field( _Number,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 3,
                                    contentCol,
                                    fieldAttributes,
                                    labelAttributes ) );

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100mTab\x1b[37m Next field\x1b[0m  \x1b[97;100mRet\x1b[37m Add\x1b[0m  \x1b[97;100mEsc\x1b[37m Cancel\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Add_Entry,
                                                   titleRow,
                                                   titleCol,
                                                   titleWidth,
                                                   false,
                                                   titleAttributes ) );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->reset();
        m_currentForm->draw();
    }
}

void Phonebook::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void Phonebook::addEntry()
{
    if( m_currentForm != nullptr )
    {
        String name( m_currentForm->getFieldContents( _Name ) );
        String number( m_currentForm->getFieldContents( _Number ) );
        m_phonebook.add( name, number );
    }
}

void Phonebook::deleteEntry()
{
    if( m_currentForm != nullptr )
    {
        String nameAndNumber( m_currentForm->getFieldContents( _Name ) );
        if( !nameAndNumber.empty() )
        {
            Tokeniser entryTokeniser( nameAndNumber, '\304' );
            String name( entryTokeniser.token() );
            name.pop_back(); // Remove trailing space
            m_phonebook.remove( name );
        }
    }
}

void Phonebook::setDefaultEntry()
{
    if( m_currentForm != nullptr )
    {
        String nameAndNumber( m_currentForm->getFieldContents( _Name ) );
        if( !nameAndNumber.empty() )
        {
            Tokeniser entryTokeniser( nameAndNumber, '\304' );
            String name( entryTokeniser.token() );
            name.pop_back(); // Remove trailing space
            m_phonebook.setDefaultEntry( name );
        }
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
