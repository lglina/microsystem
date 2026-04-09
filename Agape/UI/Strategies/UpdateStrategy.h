#ifndef AGAPE_STRATEGIES_UPDATE_H
#define AGAPE_STRATEGIES_UPDATE_H

#include "Actors/NativeActors/NativeActor.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "Promise.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

using namespace Linda2;

class InputDevice;
class Memory;
class Platform;
class Terminal;
class Timer;
class Value;
class WindowManager;

namespace UI
{

class Dialogue;

namespace Strategies
{

class Update : public Strategy, public Actors::Native
{
public:
    Update( InputDevice& inputDevice,
            WindowManager& windowManager,
            const String& windowName,
            TupleRouter& tupleRouter,
            Platform& platform,
            Agape::Memory& updateMemory,
            Dialogue& dialogue,
            Timers::Factory& timerFactory );
    ~Update();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

    virtual bool accept( Tuple& tuple );
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

private:
    enum State
    {
        none,
        notification,
        erasing,
        updating,
        error,
        success
    };

    bool needUpdate();
    bool openFile();
    bool eraseBlock();
    bool readBlock();

    void drawCheck();
    void hideDialogue();

    void drawBackground();
    void drawNotification();
    void drawProgress();
    void updateProgress();
    void drawError();
    void drawSuccess();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    TupleRouter& m_tupleRouter;
    Platform& m_platform;
    Agape::Memory& m_updateMemory;
    Dialogue& m_dialogue;
    Timers::Factory& m_timerFactory;

    enum State m_state;
    bool m_completed;

    Timer* m_timer;

    Terminal* m_terminal;

    Promise m_updateMetadataResponse;
    Promise m_updateOpenResponse;
    Promise m_updateReadResponse;

    int m_currentVersion;
    int m_newVersion;
    int m_size;
    int m_currentEraseOffset;
    int m_currentOffset;
    int m_eraseBytesPerBar;
    int m_bytesPerBar;
    int m_currentBar;
    int m_blockOffset;
    int m_blockLength;
    bool m_blockEmpty;
    Value m_blockData;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_UPDATE_H
