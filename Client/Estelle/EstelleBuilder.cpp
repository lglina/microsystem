#include "EntropySources/EstelleEntropySource.h"
#include "InputDevices/BetaKeyboard.h"
#include "Timers/Factories/PIC32PrecisionTimerFactory.h"
#include "Timers/Factories/PIC32TimerFactory.h"
#include "Timers/PIC32AbsoluteTimer.h"
#include "BatteryMonitor.h"
#include "Estelle.h"
#include "EstelleBuilder.h"
#include "Illumination.h"
#include "InterruptHandler.h"
#include "PICADC.h"
#include "PICSerial.h"
#include "PlatformController.h"
#include "ReadableWritable.h"
#include "SPIController.h"
#include "SPIEntropySender.h"
#include "SPIInputDeviceSender.h"
#include "SPIRequests.h"
#include "SPIResponder.h"
#include "Warp.h"

namespace Agape
{

Estelle* EstelleBuilder::build( PICSerial& picSerial )
{
    m_absoluteTimer = new Timers::PIC32Absolute( nullptr );

    m_timerFactory = new Timers::Factories::PIC32TimerFactory;
    ReadableWritable::setTimerFactory( m_timerFactory );
    Warp::setTimerFactory( m_timerFactory );

    m_precisionTimerFactory = new Timers::Factories::PIC32PrecisionTimerFactory;

    m_picADC = new PICADC( *m_precisionTimerFactory );

    m_keyboard = new InputDevices::BetaKeyboard( *m_timerFactory );

    m_platformController = new PlatformController( *m_keyboard,
                                                   *m_picADC,
                                                   *m_timerFactory );

    m_spiController = new SPIController( 1,
                                         100000, // Hz
                                         false, // false = slave
                                         *m_timerFactory );

    m_spiResponder = new SPIResponder( *m_spiController );

    m_spiInputDeviceSender = new SPIInputDeviceSender( *m_keyboard );
    m_spiResponder->registerResponseSource( m_spiInputDeviceSender, SPIReadInput );

    m_entropySource = new EntropySources::Estelle( *m_precisionTimerFactory, picSerial );
    m_spiEntropySender = new SPIEntropySender( *m_entropySource );
    m_spiResponder->registerResponseSource( m_spiEntropySender, SPIReadEntropy );

    m_illumination = new Illumination;
    m_spiResponder->registerResponseSource( m_illumination, SPISetIlluminationUp );
    m_spiResponder->registerResponseSource( m_illumination, SPISetIlluminationDown );
    m_spiResponder->registerResponseSource( m_illumination, SPIReadIllumination );
    m_spiResponder->registerResponseSource( m_illumination, SPISetIllumination );

    m_batteryMonitor = new BatteryMonitor( *m_picADC, *m_timerFactory );
    m_spiResponder->registerResponseSource( m_batteryMonitor, SPIReadPowerState );

    m_platformController->registerPowerControllable( m_spiController );
    m_platformController->registerPowerControllable( m_entropySource );
    m_platformController->registerPowerControllable( m_illumination );

    m_spiResponder->registerResponseSource( m_platformController, SPISetAlert );
    m_spiResponder->registerResponseSource( m_platformController, SPIReadSensors );

    return new Estelle( *m_keyboard,
                        *m_platformController,
                        *m_spiResponder,
                        *m_entropySource,
                        *m_illumination,
                        *m_batteryMonitor,
                        *m_picADC );
}

} // namespace Agape
