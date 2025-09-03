#ifndef AGAPE_ENTROPY_SOURCES_DEV_RANDOM_H
#define AGAPE_ENTROPY_SOURCES_DEV_RANDOM_H

#include "EntropySource.h"

namespace Agape
{

namespace EntropySources
{

class DevRandom : public EntropySource
{
public:
    DevRandom();

    virtual int generate( char* buffer, int len );

    virtual void run();

private:
    int m_fd;
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_DEV_RANDOM_H
