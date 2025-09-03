#include "Collections.h"
#include "InbuiltFunctions.h"
#include "String.h"
#include "Value.h"

#define _USE_MATH_DEFINES
#include <cmath>

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

bool InbuiltFunctions::perform( Value& returnValue,
                                const String& name,
                                Map< String, Value* > arguments,
                                const String& caller )
{
    if( name == "pi" )
    {
        pi( returnValue );
        return true;
    }
    else if( name == "sine" )
    {
        sine( returnValue, arguments );
        return true;
    }
    else if( name == "cosine" )
    {
        cosine( returnValue, arguments );
        return true;
    }

    return false;
}

void InbuiltFunctions::pi( Value& returnValue )
{
#ifdef M_PI
    returnValue = M_PI;
#else
    returnValue = 3.14159265359;
#endif
}

void InbuiltFunctions::sine( Value& returnValue, Map< String, Value* > arguments )
{
    returnValue = std::sin( (double)*arguments["x"] );
}

void InbuiltFunctions::cosine( Value& returnValue, Map< String, Value* > arguments )
{
    returnValue = std::cos( (double)*arguments["x"] );
}

} // namespace Carlo

} // namespace Agape
