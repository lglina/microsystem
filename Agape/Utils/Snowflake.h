#ifndef AGAPE_SNOWFLAKE_H
#define AGAPE_SNOWFLAKE_H

#include "String.h"

namespace Agape
{

class Clock;

class Snowflake
{
public:
    Snowflake( Clock& clock, int machineID );
    ~Snowflake();

    static String generate();

private:
    String _generate();

    Clock& m_clock;

    int m_machineID;
    unsigned int m_sequence;
};

} // namespace Agape

#endif // AGAPE_SNOWFLAKE_H
