#ifndef AGAPE_STRING_SERIALISER_H
#define AGAPE_STRING_SERIALISER_H

#include "ReadableWritable.h"
#include "String.h"

namespace Agape
{

class StringSerialiser : public ReadableWritable
{
public:
    StringSerialiser();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    String m_data;
    int m_offset;
};

} // namespace Agape

#endif // AGAPE_STRING_SERIALISER_H
