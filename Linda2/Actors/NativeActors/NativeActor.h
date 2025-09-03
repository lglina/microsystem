#ifndef AGAPE_LINDA2_ACTORS_NATIVE_H
#define AGAPE_LINDA2_ACTORS_NATIVE_H

#include "Actors/Actor.h"
#include "Collections.h"
#include "Value.h"

#include "String.h"

namespace Agape
{

class LiteStream;

namespace Linda2
{

class Tuple;

namespace Actors
{

class Native : public Actor
{
public:
    Native( const String& name );
    virtual ~Native() {};

    virtual bool accept( Tuple& tuple ) { return false; };
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller ) { return false; };

    virtual String actorName() const;

    virtual void str( LiteStream& stream, int indent );

protected:
    const String m_actorName;
};

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_H
