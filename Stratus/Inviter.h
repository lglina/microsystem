#ifndef AGAPE_STRATUS_INVITER
#define AGAPE_STRATUS_INVITER

#include "Actors/NativeActors/NativeActor.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Stratus
{

class Authenticator;

class Inviter : public Actors::Native
{
public:
    Inviter( TupleRouter& tupleRouter, Authenticator& authenticator );
    ~Inviter();

    virtual bool accept( Tuple& tuple );

private:
    TupleRouter& m_tupleRouter;
    Authenticator& m_authenticator;

    String m_secret;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_INVITER
