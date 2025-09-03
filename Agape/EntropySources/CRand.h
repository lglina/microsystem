#ifndef AGAPE_ENTROPY_SOURCES_C_RAND_H
#define AGAPE_ENTROPY_SOURCES_C_RAND_H

#include "EntropySource.h"

namespace Agape
{

namespace EntropySources
{

class CRand : public EntropySource
{
public:
    virtual int generate( char* buffer, int len );

    virtual void run();
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_C_RAND_H
