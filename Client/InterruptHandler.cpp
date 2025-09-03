#include "GraphicsDrivers/GraphicsDriver.h"
#include "InterruptHandler.h"

extern "C"
{
#if defined(__PIC32MX__)
    void __ISR(_UART_1_VECTOR, IPL1AUTO) interruptUART1() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART_2_VECTOR, IPL2AUTO) interruptUART2() __attribute__((__used__)) __attribute__((section(".keep"), nomips16)); // Higher prio for MIDI out.
    void __ISR(_UART_3_VECTOR, IPL2AUTO) interruptUART3() __attribute__((__used__)) __attribute__((section(".keep"), nomips16)); // Higher prio for MIDI out.
    void __ISR(_UART_4_VECTOR, IPL1AUTO) interruptUART4() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART_5_VECTOR, IPL1AUTO) interruptUART5() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI_1_VECTOR, IPL1AUTO) interruptSPI1() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI_2_VECTOR, IPL1AUTO) interruptSPI2() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
    void __ISR(_UART1_RX_VECTOR, IPL1AUTO) interruptUART1RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART1_TX_VECTOR, IPL1AUTO) interruptUART1TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART2_RX_VECTOR, IPL1AUTO) interruptUART2RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART2_TX_VECTOR, IPL1AUTO) interruptUART2TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART3_RX_VECTOR, IPL2AUTO) interruptUART3RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16)); // Higher prio for MIDI out.
    void __ISR(_UART3_TX_VECTOR, IPL2AUTO) interruptUART3TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI1_RX_VECTOR, IPL1AUTO) interruptSPI1RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI1_TX_VECTOR, IPL1AUTO) interruptSPI1TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI2_RX_VECTOR, IPL1AUTO) interruptSPI2RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_SPI2_TX_VECTOR, IPL1AUTO) interruptSPI2TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
#endif
#if defined(__PIC32MZ__)
    void __ISR(_UART4_RX_VECTOR, IPL1AUTO) interruptUART4RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART4_TX_VECTOR, IPL1AUTO) interruptUART4TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART5_RX_VECTOR, IPL1AUTO) interruptUART5RX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_UART5_TX_VECTOR, IPL1AUTO) interruptUART5TX() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
#endif
    void __ISR(_TIMER_1_VECTOR, IPL1AUTO) interruptTimer1() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));
    void __ISR(_TIMER_2_VECTOR, IPL1AUTO) interruptTimer2() __attribute__((__used__)) __attribute__((section(".keep"), nomips16));

    void _general_exception_handler(void) __attribute__((nomips16));
}

//  Exception handler: 
static enum { 
    EXCEP_IRQ = 0,          // interrupt 
    EXCEP_AdEL = 4,         // address error exception (load or ifetch) 
    EXCEP_AdES,             // address error exception (store) 
    EXCEP_IBE,              // bus error (ifetch) 
    EXCEP_DBE,              // bus error (load/store) 
    EXCEP_Sys,              // syscall 
    EXCEP_Bp,               // breakpoint 
    EXCEP_RI,               // reserved instruction 
    EXCEP_CpU,              // coprocessor unusable 
    EXCEP_Overflow,         // arithmetic overflow 
    EXCEP_Trap,             // trap (possible divide by zero) 
    EXCEP_IS1 = 16,         // implementation specfic 1 
    EXCEP_CEU,              // CorExtend Unuseable 
    EXCEP_C2E               // coprocessor 2 
} _excep_code; 

static unsigned int _epc_code;
static unsigned int _excep_addr;

// this function overrides the normal _weak_ generic handler 
void _general_exception_handler(void) 
{ 
    asm volatile("mfc0 %0,$13" : "=r" (_epc_code)); 
    asm volatile("mfc0 %0,$14" : "=r" (_excep_addr)); 

    _epc_code = (_epc_code & 0x0000007C) >> 2;
    if( Agape::InterruptDispatcher::s_graphicsDriver )
    {
        Agape::InterruptDispatcher::s_graphicsDriver->panic( _excep_addr, _epc_code );
    }

    while( 1 )
    {
    }
}// End of exception handler

#if defined(__PIC32MX__)
void __ISR(_UART_1_VECTOR, IPL1AUTO) interruptUART1()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART1 );
}

void __ISR(_UART_2_VECTOR, IPL2AUTO) interruptUART2()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART2 );
}

void __ISR(_UART_3_VECTOR, IPL2AUTO) interruptUART3()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART3 );
}

void __ISR(_UART_4_VECTOR, IPL1AUTO) interruptUART4()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART4 );
}

void __ISR(_UART_5_VECTOR, IPL1AUTO) interruptUART5()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART5 );
}

void __ISR(_SPI_1_VECTOR, IPL1AUTO) interruptSPI1()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI1 );
}

void __ISR(_SPI_2_VECTOR, IPL1AUTO) interruptSPI2()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI2 );
}

#elif defined(__PIC32MM__) || defined(__PIC32MZ__)

void __ISR(_UART1_RX_VECTOR, IPL1AUTO) interruptUART1RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART1Rx );
}

void __ISR(_UART1_TX_VECTOR, IPL1AUTO) interruptUART1TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART1Tx );
}

void __ISR(_UART2_RX_VECTOR, IPL1AUTO) interruptUART2RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART2Rx );
}

void __ISR(_UART2_TX_VECTOR, IPL1AUTO) interruptUART2TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART2Tx );
}

void __ISR(_UART3_RX_VECTOR, IPL2AUTO) interruptUART3RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART3Rx );
}

void __ISR(_UART3_TX_VECTOR, IPL2AUTO) interruptUART3TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART3Tx );
}

void __ISR(_SPI1_RX_VECTOR, IPL1AUTO) interruptSPI1RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI1Rx );
}

void __ISR(_SPI1_TX_VECTOR, IPL1AUTO) interruptSPI1TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI1Tx );
}

void __ISR(_SPI2_RX_VECTOR, IPL1AUTO) interruptSPI2RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI2Rx );
}

void __ISR(_SPI2_TX_VECTOR, IPL1AUTO) interruptSPI2TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::SPI2Tx );
}

#endif

#if defined(__PIC32MZ__)

void __ISR(_UART4_RX_VECTOR, IPL1AUTO) interruptUART4RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART4Rx );
}

void __ISR(_UART4_TX_VECTOR, IPL1AUTO) interruptUART4TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART4Tx );
}

void __ISR(_UART5_RX_VECTOR, IPL1AUTO) interruptUART5RX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART5Rx );
}

void __ISR(_UART5_TX_VECTOR, IPL1AUTO) interruptUART5TX()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::UART5Tx );
}

#endif

void __ISR(_TIMER_1_VECTOR, IPL1AUTO) interruptTimer1()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::timer1 );
}

void __ISR(_TIMER_2_VECTOR, IPL1AUTO) interruptTimer2()
{
    Agape::InterruptDispatcher::instance()->dispatch( Agape::InterruptDispatcher::timer2 );
}

namespace Agape
{

Agape::InterruptDispatcher* InterruptDispatcher::s_instance( nullptr );
Agape::GraphicsDriver* InterruptDispatcher::s_graphicsDriver( nullptr );

InterruptDispatcher::InterruptDispatcher() :
#if defined(__PIC32MX__)
  m_UART1Handler( nullptr ),
  m_UART2Handler( nullptr ),
  m_UART3Handler( nullptr ),
  m_SPI1Handler( nullptr ),
  m_SPI2Handler( nullptr ),
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
  m_UART1RxHandler( nullptr ),
  m_UART1TxHandler( nullptr ),
  m_UART2RxHandler( nullptr ),
  m_UART2TxHandler( nullptr ),
  m_UART3RxHandler( nullptr ),
  m_UART3TxHandler( nullptr ),
  m_SPI1RxHandler( nullptr ),
  m_SPI1TxHandler( nullptr ),
  m_SPI2RxHandler( nullptr ),
  m_SPI2TxHandler( nullptr ),
#endif
#if defined(__PIC32MZ__)
  m_UART4RxHandler( nullptr ),
  m_UART4TxHandler( nullptr ),
  m_UART5RxHandler( nullptr ),
  m_UART5TxHandler( nullptr ),
#endif
  m_timer1Handler( nullptr ),
  m_timer2Handler( nullptr )
{
}

void InterruptDispatcher::registerHandler( enum InterruptVector vector, InterruptHandler* handler )
{
    switch( vector )
    {
#if defined(__PIC32MX__)
    case UART1:
        m_UART1Handler = handler;
        break;
    case UART2:
        m_UART2Handler = handler;
        break;
    case UART3:
        m_UART3Handler = handler;
        break;
    case SPI1:
        m_SPI1Handler = handler;
        break;
    case SPI2:
        m_SPI2Handler = handler;
        break;
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
    case UART1Rx:
        m_UART1RxHandler = handler;
        break;
    case UART1Tx:
        m_UART1TxHandler = handler;
        break;
    case UART2Rx:
        m_UART2RxHandler = handler;
        break;
    case UART2Tx:
        m_UART2TxHandler = handler;
        break;
    case UART3Rx:
        m_UART3RxHandler = handler;
        break;
    case UART3Tx:
        m_UART3TxHandler = handler;
        break;
    case SPI1Rx:
        m_SPI1RxHandler = handler;
        break;
    case SPI1Tx:
        m_SPI1TxHandler = handler;
        break;
    case SPI2Rx:
        m_SPI2RxHandler = handler;
        break;
    case SPI2Tx:
        m_SPI2TxHandler = handler;
        break;
#endif
#if defined(__PIC32MZ__)
    case UART4Rx:
        m_UART4RxHandler = handler;
        break;
    case UART4Tx:
        m_UART4TxHandler = handler;
        break;
    case UART5Rx:
        m_UART5RxHandler = handler;
        break;
    case UART5Tx:
        m_UART5TxHandler = handler;
        break;
#endif
    case timer1:
        m_timer1Handler = handler;
        break;
    case timer2:
        m_timer2Handler = handler;
        break;
    default:
        break;
    }
}

void InterruptDispatcher::deregisterHandler( enum InterruptVector vector )
{
    switch( vector )
    {
#if defined(__PIC32MX__)
    case UART1:
        m_UART1Handler = nullptr;
        break;
    case UART2:
        m_UART2Handler = nullptr;
        break;
    case UART3:
        m_UART3Handler = nullptr;
        break;
    case SPI1:
        m_SPI1Handler = nullptr;
        break;
    case SPI2:
        m_SPI2Handler = nullptr;
        break;
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
    case UART1Rx:
        m_UART1RxHandler = nullptr;
        break;
    case UART1Tx:
        m_UART1TxHandler = nullptr;
        break;
    case UART2Rx:
        m_UART2RxHandler = nullptr;
        break;
    case UART2Tx:
        m_UART2TxHandler = nullptr;
        break;
    case UART3Rx:
        m_UART3RxHandler = nullptr;
        break;
    case UART3Tx:
        m_UART3TxHandler = nullptr;
        break;
    case SPI1Rx:
        m_SPI1RxHandler = nullptr;
        break;
    case SPI1Tx:
        m_SPI1TxHandler = nullptr;
        break;
    case SPI2Rx:
        m_SPI2RxHandler = nullptr;
        break;
    case SPI2Tx:
        m_SPI2TxHandler = nullptr;
        break;
#endif
#if defined(__PIC32MZ__)
    case UART4Rx:
        m_UART4RxHandler = nullptr;
        break;
    case UART4Tx:
        m_UART4TxHandler = nullptr;
        break;
    case UART5Rx:
        m_UART5RxHandler = nullptr;
        break;
    case UART5Tx:
        m_UART5TxHandler = nullptr;
        break;
#endif
    case timer1:
        m_timer1Handler = nullptr;
        break;
    case timer2:
        m_timer2Handler = nullptr;
        break;
    default:
        break;
    }
}

void InterruptDispatcher::dispatch( enum InterruptVector vector )
{
    switch( vector )
    {
#if defined(__PIC32MX__)
    case UART1:
        if( m_UART1Handler )
        {
            m_UART1Handler->handleInterrupt( UART1 );
        }
        break;
    case UART2:
        if( m_UART2Handler )
        {
            m_UART2Handler->handleInterrupt( UART2 );
        }
        break;
    case UART3:
        if( m_UART3Handler )
        {
            m_UART3Handler->handleInterrupt( UART3 );
        }
        break;
    case SPI1:
        if( m_SPI1Handler )
        {
            m_SPI1Handler->handleInterrupt( SPI1 );
        }
        break;
    case SPI2:
        if( m_SPI2Handler )
        {
            m_SPI2Handler->handleInterrupt( SPI2 );
        }
        break;
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
    case UART1Rx:
        if( m_UART1RxHandler )
        {
            m_UART1RxHandler->handleInterrupt( UART1Rx );
        }
        break;
    case UART1Tx:
        if( m_UART1TxHandler )
        {
            m_UART1TxHandler->handleInterrupt( UART1Tx );
        }
        break;
    case UART2Rx:
        if( m_UART2RxHandler )
        {
            m_UART2RxHandler->handleInterrupt( UART2Rx );
        }
        break;
    case UART2Tx:
        if( m_UART2TxHandler )
        {
            m_UART2TxHandler->handleInterrupt( UART2Tx );
        }
        break;
    case UART3Rx:
        if( m_UART3RxHandler )
        {
            m_UART3RxHandler->handleInterrupt( UART3Rx );
        }
        break;
    case UART3Tx:
        if( m_UART3TxHandler )
        {
            m_UART3TxHandler->handleInterrupt( UART3Tx );
        }
        break;
    case SPI1Rx:
        if( m_SPI1RxHandler )
        {
            m_SPI1RxHandler->handleInterrupt( SPI1Rx );
        }
        break;
    case SPI1Tx:
        if( m_SPI1TxHandler )
        {
            m_SPI1TxHandler->handleInterrupt( SPI1Tx );
        }
        break;
    case SPI2Rx:
        if( m_SPI2RxHandler )
        {
            m_SPI2RxHandler->handleInterrupt( SPI2Rx );
        }
        break;
    case SPI2Tx:
        if( m_SPI2TxHandler )
        {
            m_SPI2TxHandler->handleInterrupt( SPI2Tx );
        }
        break;
#endif
#if defined(__PIC32MZ__)
    case UART4Rx:
        if( m_UART4RxHandler )
        {
            m_UART4RxHandler->handleInterrupt( UART4Rx );
        }
        break;
    case UART4Tx:
        if( m_UART4TxHandler )
        {
            m_UART4TxHandler->handleInterrupt( UART4Tx );
        }
        break;
    case UART5Rx:
        if( m_UART5RxHandler )
        {
            m_UART5RxHandler->handleInterrupt( UART5Rx );
        }
        break;
    case UART5Tx:
        if( m_UART5TxHandler )
        {
            m_UART5TxHandler->handleInterrupt( UART5Tx );
        }
        break;
#endif
    case timer1:
        if( m_timer1Handler )
        {
            m_timer1Handler->handleInterrupt( timer1 );
        }
        break;
    case timer2:
        if( m_timer2Handler )
        {
            m_timer2Handler->handleInterrupt( timer2 );
        }
        break;
    default:
        break;
    }
}

InterruptDispatcher* InterruptDispatcher::instance()
{
    if( !s_instance )
    {
        s_instance = new InterruptDispatcher;
    }

    return s_instance;
}

} // namespace Agape
