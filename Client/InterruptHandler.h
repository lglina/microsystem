#ifndef AGAPE_INTERRUPT_HANDLER_H
#define AGAPE_INTERRUPT_HANDLER_H

#include <xc.h>
#include <sys/attribs.h>

#define EXCEPTION_CODE_NEW 0x10000001u
#define EXCEPTION_CODE_ALLOCATOR 0x10000002u

namespace Agape
{
    class GraphicsDriver;
    class InterruptDispatcher;
} // namespace Agape

namespace Agape
{

class InterruptHandler;

class InterruptDispatcher
{
public:
    enum InterruptVector
    {
#if defined(__PIC32MX__)
        UART1,
        UART2,
        UART3,
        UART4,
        UART5,
        SPI1,
        SPI2,
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
        UART1Rx,
        UART1Tx,
        UART2Rx,
        UART2Tx,
        UART3Rx,
        UART3Tx,
        SPI1Rx,
        SPI1Tx,
        SPI2Rx,
        SPI2Tx,
#endif
#if defined(__PIC32MZ__)
        UART4Rx,
        UART4Tx,
        UART5Rx,
        UART5Tx,
#endif
        timer1,
        timer2
    };

    InterruptDispatcher() __attribute__((section(".keep")));

    void registerHandler( enum InterruptVector vector, InterruptHandler* handler );
    void deregisterHandler( enum InterruptVector vector );
    void dispatch( enum InterruptVector vector ) __attribute__((section(".keep")));

    static InterruptDispatcher* instance();

    static InterruptDispatcher* s_instance;
    static Agape::GraphicsDriver* s_graphicsDriver;

private:
#if defined(__PIC32MX__)
    InterruptHandler* m_UART1Handler;
    InterruptHandler* m_UART2Handler;
    InterruptHandler* m_UART3Handler;
    InterruptHandler* m_UART4Handler;
    InterruptHandler* m_UART5Handler;
    InterruptHandler* m_SPI1Handler;
    InterruptHandler* m_SPI2Handler;
#elif defined(__PIC32MM__) || defined(__PIC32MZ__)
    InterruptHandler* m_UART1RxHandler;
    InterruptHandler* m_UART1TxHandler;
    InterruptHandler* m_UART2RxHandler;
    InterruptHandler* m_UART2TxHandler;
    InterruptHandler* m_UART3RxHandler;
    InterruptHandler* m_UART3TxHandler;
    InterruptHandler* m_SPI1RxHandler;
    InterruptHandler* m_SPI1TxHandler;
    InterruptHandler* m_SPI2RxHandler;
    InterruptHandler* m_SPI2TxHandler;
#endif
#if defined(__PIC32MZ__)
    InterruptHandler* m_UART4RxHandler;
    InterruptHandler* m_UART4TxHandler;
    InterruptHandler* m_UART5RxHandler;
    InterruptHandler* m_UART5TxHandler;
#endif
    InterruptHandler* m_timer1Handler;
    InterruptHandler* m_timer2Handler;
};

class InterruptHandler
{
public:
    virtual void handleInterrupt( enum InterruptDispatcher::InterruptVector vector ) = 0;
};

} // namespace Agape

#endif // AGAPE_INTERRUPT_HANDLER_H
