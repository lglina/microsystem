#include "EntropySources/EntropySource.h"
#include "InputDevices/InputDevice.h"
#include "BatteryMonitor.h"
#include "Illumination.h"
#include "PlatformController.h"
#include "SPIResponder.h"
#include "Estelle.h"

namespace Agape
{

Estelle::Estelle( InputDevice& keyboard,
                  PlatformController& platformController,
                  SPIResponder& spiResponder,
                  EntropySource& entropySource,
                  Illumination& illumination,
                  BatteryMonitor& batteryMonitor ) :
  m_keyboard( keyboard ),
  m_platformController( platformController ),
  m_spiResponder( spiResponder ),
  m_entropySource( entropySource ),
  m_illumination( illumination ),
  m_batteryMonitor( batteryMonitor )
{
}

void Estelle::run()
{
    m_keyboard.run();
    m_platformController.run();
    m_spiResponder.run();
    m_entropySource.run();
    m_illumination.run();
    m_batteryMonitor.run();
}

} // namespace Agape
