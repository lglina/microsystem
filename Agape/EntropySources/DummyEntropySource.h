#ifndef AGAPE_ENTROPY_SOURCES_DUMMY_H
#define AGAPE_ENTROPY_SOURCES_DUMMY_H

#include "EntropySources/EntropySource.h"
#include "Runnable.h"

namespace Agape
{

namespace EntropySources
{

class Dummy : public EntropySource
{
public:
    Dummy();

    virtual int generate( char* buffer, int len );

    virtual void run();

private:
    char m_nextChar;
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_DUMMY_H
