#ifndef AGAPE_PICADC_H
#define AGAPE_PICADC_H

#include "Collections.h"
#include "Runnable.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

class PICADC : public Runnable
{
public:
    PICADC( Timers::Factory& timerFactory );
    ~PICADC();

    void addChannel( int channelNum );
    bool getValue( int channelNum, int& value );

    virtual void run();

private:
    struct Channel
    {
        int m_channelNum;
        int m_value;
        bool m_haveSampled;
    };

    Timer* m_timer;

    Vector< struct Channel > m_channels;

    int m_currentChannelIdx;
};

} // namespace Agape

#endif // AGAPE_PICADC_H
