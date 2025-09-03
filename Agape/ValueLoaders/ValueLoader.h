#ifndef AGAPE_VALUE_LOADER_H
#define AGAPE_VALUE_LOADER_H

#include "Value.h"

namespace Agape
{

class ValueLoader
{
public:
    virtual ~ValueLoader() {};

    virtual bool load( Value& value ) = 0;
    virtual bool save( const Value& value ) = 0;
};

} // namespace Agape

#endif // AGAPE_VALUE_LOADER_H
