#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "World/WorldUtilities.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "ConfigurationStore.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"
#include "WorldbookStrategy.h"

using Agape::String;

namespace
{
    const int selectWidth( 50 );
    const int selectHeight( 10 );
    const int contentFirstRow( 6 );
    const int contentCol( 8 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    const int labelAttributes( 0 );

    const int titleRow( 6 );
    const int titleCol( 8 );
    const int titleWidth( 50 );
    const int titleAttributes( 0x0F );

    const int guideRow( 20 );

    const String textAssetName( "worlds-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Worldbook::Worldbook( WindowManager& windowManager,
                      const String& windowName,
                      InputDevice& inputDevice,
                      Agape::Worldbook& worldbook,
                      World::Utilities& worldUtilities,
                      ConfigurationStore& configurationStore,
                      Line& line,
                      Dialogue& dialogue ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_inputDevice( inputDevice ),
  m_worldbook( worldbook ),
  m_worldUtilities( worldUtilities ),
  m_configurationStore( configurationStore ),
  m_line( line ),
  m_dialogue( dialogue ),
  m_state( none ),
  m_completed( false ),
  m_calling( false ),
  m_currentForm( nullptr ),
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Worldbook::~Worldbook()
{
    closeForm();
}

void Worldbook::enter( const Value& parameters )
{
    m_completed = false;
    m_nextStrategy.clear();

    Line::LineStatus lineStatus( m_line.getLineStatus() );
    if( lineStatus.m_carrier )
    {
        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
        {
            m_terminal = terminalWindow.m_terminal;
        }

        m_state = select;

        drawSelectForm();
    }
    else
    {
        m_state = connect;
        m_nextStrategy = "connect";
        m_calling = true;
    }
}

void Worldbook::returnTo( const Value& parameters )
{
    if( m_state == connect )
    {
        if( (int)parameters[_success] == 1 )
        {
            m_state = select;
            m_calling = false;
            m_nextStrategy.clear();

            drawSelectForm();
        }
        else
        {
            m_calling = false;
            m_nextStrategy.clear();
            m_completed = true;
        }
    }
    else if( ( m_state == chooseAvatarCreate ) || ( m_state == chooseAvatarJoin ) )
    {
        if( (int)parameters[_success] == 1 )
        {
            m_callingParameters[_user] = parameters;

            if( m_state == chooseAvatarCreate )
            {
                m_state = create;
                m_nextStrategy = "createWorld";
                m_callingParameters[_title] = _worlds;
                if( m_configurationStore.hasKey( _accountSubKey ) )
                {
                    m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
                }
            }
            else if( m_state == chooseAvatarJoin )
            {
                m_state = join;
                m_nextStrategy = "joinWorld";
                m_callingParameters[_title] = _worlds;
                if( m_configurationStore.hasKey( _accountSubKey ) )
                {
                    m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
                }
            }
        }
        else
        {
            m_state = select;
            m_calling = false;
            m_nextStrategy.clear();
            
            drawSelectForm();
        }
    }
    else if( ( m_state == create ) || ( m_state == join ) )
    {
        if( (int)parameters[_success] == 1 )
        {
            Metadata metadata( Metadata::fromValue( parameters[_metadata], true ) ); // true = deserialise keys.
            User user( User::fromValue( parameters[_user] ) );

            bool makeDefault( !m_worldbook.hasDefaultWorldID() );
            m_worldbook.add( metadata, makeDefault );
            m_worldbook.setUserForWorld( metadata.m_worldID, user );
        }

        m_state = select;
        m_calling = false;
        m_nextStrategy.clear();

        drawSelectForm();
    }
    else if( m_state == enterWorld )
    {
        m_state = select;
        m_calling = false;
        m_nextStrategy.clear();

        drawSelectForm();
    }
    else if( m_state == loadJoined )
    {
        m_state = select;
        m_calling = false;
        m_nextStrategy.clear();

        drawSelectForm();
    }
}

bool Worldbook::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;

        return true;
    }

    return false;
}

bool Worldbook::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        nextStrategy = m_nextStrategy;
        return true;
    }
    
    return false;
}

void Worldbook::run()
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
            if( c == 'c' )
            {
                m_state = chooseAvatarCreate;
                m_nextStrategy = "chooseAvatar";
                m_callingParameters[_title] = _worlds;
                m_calling = true;
            }
            else if( c == 'j' )
            {
                m_state = chooseAvatarJoin;
                m_nextStrategy = "chooseAvatar";
                m_callingParameters[_title] = _worlds;
                m_calling = true;
            }
            else if( c == 'x' )
            {
                String worldID;
                if( getWorldID( worldID ) )
                {
                    deleteWorld( worldID );
                }

                drawSelectForm();
            }
            else if( c == 'd' )
            {
                String worldID;
                if( getWorldID( worldID ) )
                {
                    setDefaultWorldID( worldID );
                }

                drawSelectForm();
            }
            else if( c == 'l' )
            {
                m_state = loadJoined;
                m_nextStrategy = "tela";
                m_calling = true;
            }
            else if( c == '\n' )
            {
                String worldID;
                if( getWorldID( worldID ) )
                {
                    m_callingParameters[_worldID] = worldID;

                    m_state = enterWorld;
                    m_nextStrategy = "enterWorld";
                    m_calling = true;
                }
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
    }
}

void Worldbook::drawBackground()
{
    if( m_terminal != nullptr )
    {
        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void Worldbook::drawSelectForm()
{
    drawBackground();

    Vector< Metadata > worlds( m_worldbook.getAllMetadata() );
    String defaultID = m_worldbook.getDefaultWorldID();
    String catWorldNames;
    m_worldIDs.clear();
    Vector< Metadata >::const_iterator it( worlds.begin() );
    for( ; it != worlds.end(); ++it )
    {
        if( it != worlds.begin() )
        {
            catWorldNames += ';';
        }

        catWorldNames += it->m_name;

        if( it->m_worldID == defaultID )
        {
            catWorldNames += " (default)";
        }

        m_worldIDs.push_back( it->m_worldID );
    }

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _name,
                                    selectWidth,
                                    selectHeight,
                                    contentFirstRow,
                                    contentCol,
                                    fieldAttributes,
                                    catWorldNames,
                                    selectionAttributes,
                                    labelAttributes ) );

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mC\x1b[37m Create\x1b[0m  \x1b[97;100mJ\x1b[37m Join\x1b[0m  \x1b[97;100mX\x1b[37m Delete\x1b[0m  \x1b[97;100mD\x1b[37m Set default\x1b[0m  \x1b[97;100mEsc\x1b[37m Menu\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title( _worlds,
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

void Worldbook::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

bool Worldbook::getWorldID( String& worldID )
{
    if( m_currentForm != nullptr )
    {
        int idx( m_currentForm->getSelectIndex( _name ) );
        if( ( idx >= 0 ) && ( idx < m_worldIDs.size() ) )
        {
            worldID = m_worldIDs[idx];
            return true;
        }
    }

    return false;
}

void Worldbook::deleteWorld( const String& worldID )
{
    m_worldbook.remove( worldID );
    m_worldbook.removeUserForWorld( worldID );
}

void Worldbook::setDefaultWorldID( const String& worldID )
{
    m_worldbook.setDefaultWorldID( worldID );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
