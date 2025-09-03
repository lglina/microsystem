#ifndef AGAPE_SESSION_H
#define AGAPE_SESSION_H

#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace UI
{
class Strategy;
} // namespace UI

class Session : public Runnable
{
public:
    Session( Map< String, UI::Strategy* > strategies,
             const String& initialStrategy );
    virtual ~Session() {}

    virtual void run();

private:
    Map< String, UI::Strategy* > m_strategies;
    String m_initialStrategy;

    Deque< UI::Strategy* > m_stack;
    bool m_started;
};

} // namespace Agape

#endif // AGAPE_SESSION_H
