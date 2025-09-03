#ifndef AGAPE_STRATUS_AUTHENTICATOR_H
#define AGAPE_STRATUS_AUTHENTICATOR_H

#include "Actors/NativeActors/NativeActor.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

class KeyUtilities;

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

class Value;

using namespace Linda2;

namespace Stratus
{

class Authenticator : public Actors::Native
{
public:
    Authenticator( TupleRouter& tupleRouter, KeyUtilities& keyUtilities );
    virtual ~Authenticator();

    virtual bool accept( Tuple& tuple );

    bool credentialsValid() const;
    bool isTela() const;

    const String& accountAuthKeyHash() const;
    const String& deviceAuthKeyHash() const;

    bool joinedWorld( const String& worldID );
    bool writableWorld( const String& worldID );

    bool isOurUser( const String& snowflake ) const;

private:
    void validateCredentials();
    void cacheUserSnowflakes();
    void hasJoinedWorld( const String& worldID, bool& joined, bool& writable );
    void hasJoinedWorldWithDevice( const Value* deviceValue, const String& worldID, bool& joined, bool& writable );

    TupleRouter& m_tupleRouter;
    KeyUtilities& m_keyUtilities;

    String m_accountAuthKeyHash;
    String m_deviceAuthKeyHash;

    bool m_credentialsValid;
    bool m_isTela;

    Vector< String > m_joinedWorldIDs;
    Vector< String > m_writableWorldIDs;

    Vector< String > m_userSnowflakes;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_AUTHENTICATOR_H
