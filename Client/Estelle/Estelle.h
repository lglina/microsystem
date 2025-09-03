#ifndef AGAPE_ESTELLE_H
#define AGAPE_ESTELLE_H

namespace Agape
{

class BatteryMonitor;
class EntropySource;
class Illumination;
class InputDevice;
class PlatformController;
class SPIResponder;

class Estelle
{
public:
    Estelle( InputDevice& keyboard,
             PlatformController& platformController,
             SPIResponder& spiResponder,
             EntropySource& entropySource,
             Illumination& illumination,
             BatteryMonitor& batteryMonitor );

    void run();

private:
    EntropySource& m_entropySource;
    InputDevice& m_keyboard;
    PlatformController& m_platformController;
    SPIResponder& m_spiResponder;
    Illumination& m_illumination;
    BatteryMonitor& m_batteryMonitor;
};

} // namespace Agape

#endif // AGAPE_ESTELLE_H
