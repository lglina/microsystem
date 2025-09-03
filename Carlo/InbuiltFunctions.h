#ifndef AGAPE_CARLO_INBUILT_FUNCTIONS_H
#define AGAPE_CARLO_INBUILT_FUNCTIONS_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

class Value;

namespace Carlo
{

class InbuiltFunctions
{
public:
    bool perform( Value& returnValue,
                  const String& name,
                  Map< String, Value* > arguments,
                  const String& caller );

private:
    void pi( Value& returnValue );

    void sine( Value& returnValue, Map< String, Value* > arguments );
    void cosine( Value& returnValue, Map< String, Value* > arguments );
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_INBUILT_FUNCTIONS_H
