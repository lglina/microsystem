#ifndef ESTELLE_BUILDER_H
#define ESTELLE_BUILDER_H

namespace Agape
{

namespace EntropySources
{
class Estelle;
} // namespace EntropySources

namespace InputDevices
{
class BetaKeyboard;
} // namespace InputDevices

namespace Timers
{
class Factory;
class PIC32Absolute;
} // namespace Timers

class BatteryMonitor;
class Illumination;
class PICSerial;
class PlatformController;
class SPIController;
class SPIEntropySender;
class SPIInputDeviceSender;
class SPIResponder;

class EstelleBuilder
{
public:
    virtual Estelle* build( PICSerial& picSerial );

private:
    Timers::PIC32Absolute* m_absoluteTimer;
    Timers::Factory* m_timerFactory;
    Timers::Factory* m_precisionTimerFactory;
    InputDevices::BetaKeyboard* m_keyboard;
    PlatformController* m_platformController;
    SPIController* m_spiController;
    SPIResponder* m_spiResponder;
    SPIInputDeviceSender* m_spiInputDeviceSender;
    EntropySources::Estelle* m_entropySource;
    SPIEntropySender* m_spiEntropySender;
    Illumination* m_illumination;
    BatteryMonitor* m_batteryMonitor;
};

} // namespace Agape

#endif // ESTELLE_BUILDER_H
