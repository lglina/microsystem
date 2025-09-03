#include "Loggers/Logger.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "Session.h"
#include "String.h"
#include "Value.h"

namespace
{
    const char* fallbackStrategy( "menu" );
} // Anonymous namespace

namespace Agape
{

Session::Session( Map< String, UI::Strategy* > strategies,
                  const String& initialStrategy ) :
  m_strategies( strategies ),
  m_initialStrategy( initialStrategy ),
  m_started( false )
{
}

void Session::run()
{
    if( !m_started )
    {
        LOG_DEBUG( "Session: Initial strategy " + m_initialStrategy );
        Map< String, UI::Strategy* >::const_iterator it( m_strategies.find( m_initialStrategy ) );
        if( it != m_strategies.end() )
        {
            m_stack.push_back( it->second );
            Value parameters;
            m_stack.back()->enter( parameters );
        }
        else
        {
            LOG_DEBUG( "Session: Initial strategy not found. Unable to proceed." );
        }
        m_started = true;
    }

    if( !m_stack.empty() )
    {
        UI::Strategy* currentStrategy( m_stack.back() );

        currentStrategy->run();

        String strategyName;
        String nextStrategy;
        Value value;
        if( currentStrategy->calling( strategyName, value ) )
        {
            LOG_DEBUG( "Session: Calling " + strategyName );
            Map< String, UI::Strategy* >::const_iterator it( m_strategies.find( strategyName ) );
            if( it != m_strategies.end() )
            {
                m_stack.push_back( it->second );
                m_stack.back()->enter( value );
            }
            else
            {
                LOG_DEBUG( "Session: Called strategy not found" );
                m_stack.back()->returnTo( value );
            }
        }
        else if( currentStrategy->returning( nextStrategy, value ) )
        {
            m_stack.pop_back();

            if( nextStrategy.empty() && m_stack.empty() )
            {
                // Fallback strategy
                LOG_DEBUG( "Session: Returned and no next strategy. Falling back to menu." );
                nextStrategy = fallbackStrategy;
            }

            bool enteredNext( false );
            if( !nextStrategy.empty() )
            {
                LOG_DEBUG( "Session: Returning to " + nextStrategy );
                Map< String, UI::Strategy* >::const_iterator it( m_strategies.find( nextStrategy ) );
                if( it != m_strategies.end() )
                {
                    m_stack.push_back( it->second );
                    m_stack.back()->enter( value );
                    enteredNext = true;
                }
                else
                {
                    if( !m_stack.empty() )
                    {
                        LOG_DEBUG( "Session: Requested return strategy not found. Returning to caller." );
                        m_stack.back()->returnTo( value );
                    }
                    else
                    {
                        LOG_DEBUG( "Session: Requested return strategy not found and no caller. Unable to proceed." );
                    }
                }
            }

            if( !enteredNext && !m_stack.empty() )
            {
                Map< String, UI::Strategy* >::const_iterator it( m_strategies.begin() );
                for( ; it != m_strategies.end(); ++it )
                {
                    if( it->second == m_stack.back() )
                    {
                        LOG_DEBUG( "Session: Returning to " + it->first );
                        break;
                    }
                }

                currentStrategy = m_stack.back();
                currentStrategy->returnTo( value );
            }
        }
    }
}

} // namespace Agape
