#ifndef AGAPE_CLIENT_H
#define AGAPE_CLIENT_H

namespace Agape
{

namespace Audio
{
class MIDIPlayer;
} // namespace Audio

using namespace Audio;

namespace Linda2
{
namespace Actors
{
namespace NativeActors
{
class EventTimer;
} // namespace NativeActors
} // namespace Actors
} // namespace Linda2

namespace UI
{
class PlatformUI;
} // namespace UI

class ConnectionMonitor;
class EventClock;
class InputDevice;
class EntropySource;
class Line;
class Platform;
class Session;
class String;

class Client
{
public:
    // UIs are usually called by the active UI strategy as needed, but for
    // UIs which we want to see in all strategies, they can implement
    // Runnable and we call them in Client.

    Client( Line& line,
            ConnectionMonitor& connectionMonitor,
            Platform& platform,
            UI::PlatformUI& platformUI,
            InputDevice& inputDevice,
            EntropySource& entropySource,
            MIDIPlayer& midiPlayer,
            EventClock& eventClock,
            Linda2::Actors::NativeActors::EventTimer& eventTimer,
            Session& session );

    void showAssetModal( const String& assetName );

    void run();

private:
    Line& m_line;
    ConnectionMonitor& m_connectionMonitor;
    Platform& m_platform;
    UI::PlatformUI& m_platformUI;
    InputDevice& m_inputDevice;
    EntropySource& m_entropySource;
    MIDIPlayer& m_midiPlayer;
    EventClock& m_eventClock;
    Linda2::Actors::NativeActors::EventTimer& m_eventTimer;
    Session& m_session;
};

} // namespace Agape

#endif // AGAPE_CLIENT_H
