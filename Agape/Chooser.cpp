#include "Loggers/Logger.h"
#include "Chooser.h"
#include "ClientBuilder.h"
#include "Collections.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace Agape
{

Chooser::Chooser() :
  m_currentChoice( m_builders.end() ),
  m_changed( false )
{
}

Chooser::~Chooser()
{
    LOG_DEBUG( "Chooser: Destructing" );
    Map< String, ClientBuilder* >::iterator it( m_builders.begin() );
    for( ; it != m_builders.end(); ++it )
    {
        LOG_DEBUG( "Chooser: Destructing builder " + it->first );
        delete( it->second );
    }
}

void Chooser::addChoice( ClientBuilder* clientBuilder, const String& name )
{
    m_builders[name] = clientBuilder;

    if( m_currentChoice == m_builders.end() )
    {
        m_currentChoice = m_builders.begin();
    }
}

bool Chooser::setCurrentChoice( const String& name )
{
    Map< String, ClientBuilder* >::const_iterator choice( m_builders.find( name ) );
    if( choice != m_builders.end() )
    {
        m_currentChoice = choice;
        return true;
    }

    return false;
}

const String& Chooser::currentChoiceName()
{
    if( m_currentChoice != m_builders.end() )
    {
        return m_currentChoice->first;
    }

    return String();
}

ClientBuilder* Chooser::currentChoice()
{
    if( m_currentChoice != m_builders.end() )
    {
        return m_currentChoice->second;
    }

    return nullptr;
}

void Chooser::nextChoice()
{
    if( !m_builders.empty() )
    {
        ++m_currentChoice;
        if( m_currentChoice == m_builders.end() )
        {
            m_currentChoice = m_builders.begin();
        }

        m_changed = true;
    }
}

bool Chooser::changed()
{
    return m_changed;
}

void Chooser::resetChanged()
{
    m_changed = false;
}

} // namespace Agape
