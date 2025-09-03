#ifndef AGAPE_ENTROPY_SOURCE_H
#define AGAPE_ENTROPY_SOURCE_H

#include "Runnable.h"

namespace Agape
{

class EntropySource : public Runnable
{
public:
    virtual ~EntropySource() {}
    
    virtual int generate( char* buffer, int len ) = 0;

    virtual int poolSize() { return 0; };
    virtual int poolRemain() { return 0; };

    virtual void run() = 0;
};

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCE_H
