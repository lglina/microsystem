#ifndef AGAPE_ILLUMINATION_H
#define AGAPE_ILLUMINATION_H

#include "PowerControllable.h"
#include "Runnable.h"
#include "SPIResponder.h"

namespace Agape
{

class Illumination : public Runnable, public PowerControllable, public SPIResponseSource
{
public:
    Illumination();

    virtual void run();

    virtual void setPowerState( enum PowerState powerState );

    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength );
};

} // namespace Agape

#endif // AGAPE_ILLUMINATION_H
