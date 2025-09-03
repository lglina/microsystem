#ifndef AGAPE_STRATEGY_H
#define AGAPE_STRATEGY_H

#include "Runnable.h"
#include "String.h"

namespace Agape
{

class Value;

namespace UI
{

class Strategy : public Runnable
{
public:
    virtual ~Strategy() {};

    virtual void enter( const Value& parameters ) = 0;
    virtual void returnTo( const Value& parameters ) = 0;

    virtual bool calling( String& strategyName, Value& parameters ) = 0;
    virtual bool returning( String& nextStrategy, Value& parameters ) = 0;

    virtual bool needsFocus() { return false; };

    virtual void run() = 0;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGY_H
