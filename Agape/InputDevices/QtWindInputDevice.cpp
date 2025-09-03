#include "QtWindInputDevice.h"

#include <deque>
#include <QKeyEvent>

#include <iomanip>
#include <iostream>

namespace Agape
{

namespace InputDevices
{

void QtWind::consumeKeyPress( QKeyEvent* event )
{
    if( event->key() < 256 )
    {
        if( ( event->modifiers() & Qt::ControlModifier ) &&
            ( event->key() >= 'A' ) && ( event->key() <= 'Z' ) )
        {
            m_buffer.push_back( control( event->key() + ( 'a' - 'A' ) ) ); // Downcase and set MSb (control flag).
        }
        else if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( event->key() ) );
        }
        else
        {
            m_buffer.push_back( event->text()[0].toLatin1() );
        }
    }
    else if( event->key() == Qt::Key_Tab )
    {
        m_buffer.push_back( Key::tab );
    }
    else if( event->key() == Qt::Key_Backtab )
    {
        m_buffer.push_back( Key::shiftTab );
    }
    else if( event->key() == Qt::Key_Return )
    {
        m_buffer.push_back( '\r' );
        m_buffer.push_back( '\n' );
    }
    else if( event->key() == Qt::Key_Up )
    {
        if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( Key::up ) );
        }
        else if( event->modifiers() & Qt::ShiftModifier )
        {
            m_buffer.push_back( Key::shiftUp );
        }
        else
        {
            m_buffer.push_back( Key::up );
        }
    }
    else if( event->key() == Qt::Key_Down )
    {
        if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( Key::down ) );
        }
        else if( event->modifiers() & Qt::ShiftModifier )
        {
            m_buffer.push_back( Key::shiftDown );
        }
        else
        {
            m_buffer.push_back( Key::down );
        }
    }
    else if( event->key() == Qt::Key_Left )
    {
        if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( Key::left ) );
        }
        else if( event->modifiers() & Qt::ShiftModifier )
        {
            m_buffer.push_back( Key::shiftLeft );
        }
        else
        {
            m_buffer.push_back( Key::left );
        }
    }
    else if( event->key() == Qt::Key_Right )
    {
        if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( Key::right ) );
        }
        else if( event->modifiers() & Qt::ShiftModifier )
        {
            m_buffer.push_back( Key::shiftRight );
        }
        else
        {
            m_buffer.push_back( Key::right );
        }
    }
    else if( event->key() == Qt::Key_Backspace )
    {
        if( event->modifiers() & Qt::ControlModifier )
        {
            m_buffer.push_back( control( Key::backspace ) );
        }
        else
        {
            m_buffer.push_back( Key::backspace );
        }
    }
    else if( event->key() == Qt::Key_Escape )
    {
        m_buffer.push_back( Key::escape );
    }
    else
    {
        std::cerr << "Unknown extended key code " << event->key() << std::endl;
    }
}

bool QtWind::eof()
{
    return m_buffer.empty();
}

char QtWind::peek()
{
    if( !m_buffer.empty() )
    {
        return m_buffer.front();
    }

    return '\0';
}

char QtWind::get()
{
    char c( m_buffer.front() );
    m_buffer.pop_front();
    return c;
}

void QtWind::run()
{
}

} // namespace InputDevices

} // namespace Agape
