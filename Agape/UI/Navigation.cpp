#include "Utils/LiteStream.h"
#include "World/WorldMetadata.h"
#include "World/WorldCoordinates.h"
#include "Navigation.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace Agape
{

using namespace World;

namespace UI
{

Navigation::Navigation( WindowManager& windowManager,
                        const String& windowName,
                        const Coordinates& coordinates,
                        const Metadata& worldMetadata ) :
  m_coordinates( coordinates ),
  m_worldMetadata( worldMetadata ),
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

void Navigation::draw()
{
    draw( m_coordinates );
}

void Navigation::draw( const Coordinates& coordinates )
{
    if( !m_terminal ) return;

    m_terminal->clearScreen(); // FIXME: Could just backspace?

    LiteStream stream;
    stream << m_worldMetadata.m_name << " ";
    stream << std::abs( coordinates.m_y ) << " ";
    if( coordinates.m_y >= 0 )
    {
        stream << "North, ";
    }
    else
    {
        stream << "South, ";
    }

    stream << std::abs( coordinates.m_x ) << " ";
    if( coordinates.m_x >= 0 )
    {
        stream << "East.";
    }
    else
    {
        stream << "West.";
    }

    m_terminal->consumeString( stream.str() );
}

void Navigation::clear()
{
    if( !m_terminal ) return;

    m_terminal->clearScreen(); // FIXME: Could just backspace?
}

} // namespace UI

} // namespace Agape
