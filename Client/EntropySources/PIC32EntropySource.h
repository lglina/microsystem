#ifndef AGAPE_ENTROPY_SOURCES_PIC32_H
#define AGAPE_ENTROPY_SOURCES_PIC32_H

#include "EntropySources/EntropySource.h"

namespace Agape
{

namespace EntropySources
{

class PIC32TRNG : public EntropySource
{
public:
    PIC32TRNG();

    virtual int generate( char* buffer, int len );

    virtual int poolSize();
    virtual int poolRemain();

    virtual void run();
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_PIC32_H
