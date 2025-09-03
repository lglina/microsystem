#include "Audio/MIDIPlayer.h"
#include "EntropySources/EntropySource.h"
#include "EventClocks/EventClock.h"
#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "NativeActors/EventTimerActor.h"
#include "Platforms/Platform.h"
#include "UI/PlatformUI.h"
#include "Client.h"
#include "ConnectionMonitor.h"
#include "Session.h"

namespace Agape
{

Client::Client( Line& line,
                ConnectionMonitor& connectionMonitor,
                Platform& platform,
                UI::PlatformUI& platformUI,
                InputDevice& inputDevice,
                EntropySource& entropySource,
                MIDIPlayer& midiPlayer,
                EventClock& eventClock,
                Linda2::Actors::NativeActors::EventTimer& eventTimer,
                Session& session ) :
  m_line( line ),
  m_connectionMonitor( connectionMonitor ),
  m_platform( platform ),
  m_platformUI( platformUI ),
  m_inputDevice( inputDevice ),
  m_entropySource( entropySource ),
  m_midiPlayer( midiPlayer ),
  m_eventClock( eventClock ),
  m_eventTimer( eventTimer ),
  m_session( session )
{
}

void Client::showAssetModal( const String& assetName )
{
    m_platformUI.showAssetModal( assetName );
}

void Client::run()
{
    m_line.run();
    m_connectionMonitor.run();
    m_platform.run();
    m_platformUI.run();
    m_inputDevice.run();
    m_entropySource.run();
    m_midiPlayer.run();
    m_eventClock.run();
    m_eventTimer.run();
    m_session.run();
}

} // namespace Agape
