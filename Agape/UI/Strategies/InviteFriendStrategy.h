#ifndef AGAPE_STRATEGIES_INVITE_FRIEND_H
#define AGAPE_STRATEGIES_INVITE_FRIEND_H

#include "Actors/NativeActors/NativeActor.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "Promise.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{
class TupleRouter;
class Tuple;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Metadata;
} // namespace World

using namespace Linda2;
using namespace World;

class InputDevice;
class KeyUtilities;
class LiteStream;
class Terminal;
class Timer;
class Value;
class WindowManager;
class Worldbook;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Dialogue;

namespace Strategies
{

class InviteFriend : public Strategy, public Actors::Native
{
public:
    InviteFriend( InputDevice& inputDevice,
                  Worldbook& worldbook,
                  const Metadata& worldMetadata,
                  KeyUtilities& keyUtilities,
                  TupleRouter& tupleRouter,
                  WindowManager& windowManager,
                  const String& windowName,
                  Dialogue& dialogue,
                  Timers::Factory& timerFactory );
    
    ~InviteFriend();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

    virtual void str( LiteStream& stream, int indent );

    virtual bool accept( Tuple& tuple );
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

    virtual String actorName() const;

private:
    enum State
    {
        none,
        enterDetails,
        success,
        error
    };

    void drawBackground();

    void drawEnterDetails();
    void drawPending();
    void drawSuccess();
    void drawError();

    bool formComplete();

    void hideDialogue();

    void closeForm();

    bool doInvite();

    InputDevice& m_inputDevice;
    Worldbook& m_worldbook;
    const Metadata& m_worldMetadata;
    KeyUtilities& m_keyUtilities;
    TupleRouter& m_tupleRouter;
    WindowManager& m_windowManager;
    String m_windowName;
    Dialogue& m_dialogue;
    Timers::Factory& m_timerFactory;

    Timer* m_timer;

    Terminal* m_terminal;

    Forms::Form* m_currentForm;

    enum State m_state;
    bool m_completed;

    Promise m_inviteFriendResponse;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_INVITE_FRIEND_H
