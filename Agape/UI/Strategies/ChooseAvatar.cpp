#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "EntropySources/EntropySource.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "ChooseAvatar.h"
#include "Collections.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;
using namespace Agape::InputDevices;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int fieldAttributes( 0x07 );
    const int textWidth( 50 );
    const int textHeight( 1 );

    const String backgroundAssetName( "menu-wide" );
    const String welcomeTextAssetName( "welcome-text" );
    const String worldsTextAssetName( "worlds-text" );

    const int checkerRow( 10 );
    const int checkerFirstCol( 14 );
    const int checkerSpacing( 14 );

    const int avatarCharBase( 128 );
    const int avatarCharWidth( 9 );
    
    const char avatarDefaultAttributes( 0x07 );

    const int wanderDelay( 250 );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

ChooseAvatar::ChooseAvatar( InputDevice& inputDevice,
                            WindowManager& windowManager,
                            const String& windowName,
                            EntropySource& entropySource,
                            Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_entropySource( entropySource ),
  m_timer( timerFactory.makeTimer() ),
  m_state( none ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_currentForm( nullptr ),
  m_avatarIndex( 0 ),
  m_avatarColour( 0x0F ),
  m_avatarAnimation( 0 )
{
}

ChooseAvatar::~ChooseAvatar()
{
    closeForm();
    delete( m_timer );
}

void ChooseAvatar::enter( const Value& parameters )
{
    m_completed = false;

    m_parameters = parameters;
    
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
        m_state = chooseAvatar;
        m_timer->reset();
        drawChooseAvatar();
    }
    else
    {
        m_returnParameters[_success] = 0;
        m_completed = true; // Uh oh!
    }
}

void ChooseAvatar::returnTo( const Value& parameters )
{
}

bool ChooseAvatar::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool ChooseAvatar::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        m_terminal->deleteCursor( _Avatar1 );
        m_terminal->deleteCursor( _Avatar2 );
        m_terminal->deleteCursor( _Avatar3 );
        m_terminal->deleteCursor( _Avatar4 );

        closeForm();

        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void ChooseAvatar::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }
    
        switch( m_state )
        {
        case chooseAvatar:
            if( c == Key::left )
            {
                --m_avatarIndex;
                if( m_avatarIndex < 0 )
                {
                    m_avatarIndex = 3;
                }
                switchAvatar();
            }
            else if( c == Key::right )
            {
                ++m_avatarIndex;
                if( m_avatarIndex > 3 )
                {
                    m_avatarIndex = 0;
                }
                switchAvatar();
            }
            else if( c == Key::down )
            {
                --m_avatarColour;
                if( m_avatarColour < 0 )
                {
                    m_avatarColour = 15;
                }
                switchAvatarColour();
            }
            else if( c == Key::up )
            {
                ++m_avatarColour;
                if( m_avatarColour > 15 )
                {
                    m_avatarColour = 0;
                }
                switchAvatarColour();
            }
            else if( c == '\n' )
            {
                m_state = chooseName;
                drawChooseName();
            }
            else if( c == '\x1b' )
            {
                m_returnParameters[_success] = 0;
                m_completed = true;
            }
            break;
        case chooseName:
            if( c == '\n' )
            {
                m_returnParameters[_glyph] = avatarCharBase + ( avatarCharWidth * m_avatarIndex );
                m_returnParameters[_attributes] = m_avatarColour;

                if( m_currentForm != nullptr )
                {
                    m_returnParameters[_name] = m_currentForm->getFieldContents( _name );
                }

                m_returnParameters[_success] = 1;
                m_completed = true;
            }
            else if( c == '\x1b' )
            {
                m_state = chooseAvatar;
                m_timer->reset();
                drawChooseAvatar();
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        default:
            break;
        }
    }

    if( m_state == chooseAvatar )
    {
        avatarWander();
    }
}

void ChooseAvatar::drawBackground()
{
    if( m_terminal != nullptr )
    {
        String textAssetName;
        if( m_parameters[_title] == String( _welcome ) )
        {
            textAssetName = welcomeTextAssetName;
        }
        else if( m_parameters[_title] == String( _worlds ) )
        {
            textAssetName = worldsTextAssetName;
        }

        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void ChooseAvatar::drawChooseAvatar()
{
    closeForm();
    
    drawBackground();

    AssetLoaders::Baked assetLoader( World::Coordinates(), _checker );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        for( int i = 0; i < 4; ++i )
        {
            m_terminal->consumeNext( checkerRow, checkerFirstCol + ( checkerSpacing * i ) );
            m_terminal->consumeAsset( ansiFile, 0, ansiFile.dataSize(), ansiFile.width(), checkerFirstCol + ( checkerSpacing * i ), Terminal::noMaxRow );
        }
    }

    const char* message( "Choose how you want to look, and what people should call you.\
                          First, use the \x1b[97mleft\x1b[0m and \x1b[97mright\x1b[0m keys to choose an avatar, and the\
                          \x1b[97mup and \x1b[97mdown\x1b[0m keys to select a colour, then hit \x1b[97mRet\x1b[0m (Return/Enter)." );

    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentFirstCol,
                                3, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    m_terminal->createCursor( _Avatar1, checkerRow + 2, checkerFirstCol + 5, avatarCharBase, m_avatarColour, Terminal::avatarCharset );
    m_terminal->createCursor( _Avatar2, checkerRow + 2, checkerFirstCol + ( checkerSpacing * 1 ) + 5, avatarCharBase + avatarCharWidth, avatarDefaultAttributes, Terminal::avatarCharset );
    m_terminal->createCursor( _Avatar3, checkerRow + 2, checkerFirstCol + ( checkerSpacing * 2 ) + 5, avatarCharBase + ( avatarCharWidth * 2 ), avatarDefaultAttributes, Terminal::avatarCharset );
    m_terminal->createCursor( _Avatar4, checkerRow + 2, checkerFirstCol + ( checkerSpacing * 3 ) + 5, avatarCharBase + ( avatarCharWidth * 3 ), avatarDefaultAttributes, Terminal::avatarCharset );
}

void ChooseAvatar::drawChooseName()
{
    const String message( "Now enter the name you want to call yourself, then hit \x1b[97mRet\x1b[0m.\
                           This doesn't need to be your real name - use whatever you like." );

    m_terminal->printFormatted( message,
                                contentFirstRow + 10,
                                contentFirstCol,
                                2, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _name, textWidth, textHeight, contentFirstRow + 13, contentFirstCol, fieldAttributes ) );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void ChooseAvatar::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void ChooseAvatar::avatarWander()
{
    if( m_timer->ms() >= wanderDelay )
    {
        m_timer->reset();

        ++m_avatarAnimation;
        if( m_avatarAnimation == 2 )
        {
            m_avatarAnimation = 0;
        }

        switch( m_avatarIndex )
        {
        case 0:
            randomMove( _Avatar1, checkerRow, checkerRow + 4, checkerFirstCol, checkerFirstCol + 11, avatarCharBase, m_avatarAnimation );
            break;
        case 1:
            randomMove( _Avatar2, checkerRow, checkerRow + 4, checkerFirstCol * 2, ( checkerFirstCol * 2 ) + 11, avatarCharBase + avatarCharWidth, m_avatarAnimation );
            break;
        case 2:
            randomMove( _Avatar3, checkerRow, checkerRow + 4, checkerFirstCol * 3, ( checkerFirstCol * 3 ) + 11, avatarCharBase + ( avatarCharWidth * 2 ), m_avatarAnimation );
            break;
        case 3:
            randomMove( _Avatar4, checkerRow, checkerRow + 4, checkerFirstCol * 4, ( checkerFirstCol * 4 ) + 11, avatarCharBase + ( avatarCharWidth * 3 ), m_avatarAnimation );
            break;
        default:
            break;
        }
    }
}

void ChooseAvatar::switchAvatar()
{
    int row, col;
    String cursorName( avatarCursorName( m_avatarIndex ) );
    if( !m_terminal->getCursor( cursorName, row, col ) ) return;
    m_terminal->moveCursor( cursorName, row, col, avatarCharBase + ( avatarCharWidth * m_avatarIndex ), m_avatarColour );

    for( int i = 0; i < 4; ++i )
    {
        if( i != m_avatarIndex )
        {
            cursorName = avatarCursorName( i );
            m_terminal->getCursor( cursorName, row, col );
            m_terminal->moveCursor( cursorName, row, col, avatarCharBase + ( avatarCharWidth * i ), avatarDefaultAttributes );
        }
    }
}

void ChooseAvatar::switchAvatarColour()
{
    int row, col;
    String cursorName( avatarCursorName( m_avatarIndex ) );
    if( !m_terminal->getCursor( cursorName, row, col ) ) return;
    m_terminal->moveCursor( cursorName, row, col, avatarCharBase + ( avatarCharWidth * m_avatarIndex ), m_avatarColour );
}

String ChooseAvatar::avatarCursorName( int avatarIndex )
{
    switch( avatarIndex )
    {
    case 0:
        return _Avatar1;
        break;
    case 1:
        return _Avatar2;
        break;
    case 2:
        return _Avatar3;
        break;
    case 3:
        return _Avatar4;
        break;
    default:
        break;
    }

    return String();
}

void ChooseAvatar::randomMove( const String& cursorName, int y1, int y2, int x1, int x2, int charBase, int animation )
{
    int row, col;
    if( !m_terminal->getCursor( cursorName, row, col ) ) return;

    char random;
    m_entropySource.generate( &random, 1 );
    random %= 5;

    if( random == 0 && row - 1 >= y1 )
    {
        m_terminal->moveCursor( cursorName, row - 1, col, charBase + 1 + animation );
    }
    else if( random == 1 && row + 1 <= y2 )
    {
        m_terminal->moveCursor( cursorName, row + 1, col, charBase + 3 + animation );
    }
    else if( random == 2 && col - 1 >= x1 )
    {
        m_terminal->moveCursor( cursorName, row, col - 1, charBase + 5 + animation );
    }
    else if( random == 3 && col + 1 <= x2 )
    {
        m_terminal->moveCursor( cursorName, row, col + 1, charBase + 7 + animation );
    }
    else
    {
        m_terminal->moveCursor( cursorName, row, col, charBase );
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
