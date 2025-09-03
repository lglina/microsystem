#include "RandomReadableWritable.h"

namespace Agape
{

int RandomReadableWritable::readAll( char* data, int offset, int len )
{
    int numThisRead( read( data, offset, len ) );
    int numRead = numThisRead;
    int numRemain( len - numThisRead );
    while( ( numThisRead > 0 ) && ( numRemain > 0 ) )
    {
        numThisRead = read( data + numRead, offset + numRead, numRemain );
        numRead += numThisRead;
        numRemain -= numThisRead;
    }

    return( numRead );
}

} // namespace Agape
