#include "String.h"
#include "TextSquasher.h"

namespace Agape
{

namespace Encryptors
{

namespace Utils
{

namespace BIP39
{

String squashEightToFive( const String& word )
{
    // Make the base character '_' as it's below 'a' and squashed words will
    // remain in the proper sorted order.
    String input( word );
    input.append( 8 - word.length(), '_' );
    String output( 5, '\0' );
    int charnum( 0 );
    int bitpos( 0 );
    for( int i = 0; i < 8; ++i )
    {
        if( bitpos <= 3 ) output[charnum] += ( input[i] - '_' ) << ( 3 - bitpos );
        if( bitpos > 3 ) output[charnum] += ( input[i] - '_' ) >> ( bitpos - 3 );
        bitpos += 5;
        if( bitpos > 8 )
        {
            bitpos -= 8;
            output[charnum + 1] += ( input[i] - '_' ) << ( 8 - bitpos );
            ++charnum;
        }
    }

    return output;
}

String unsquashFiveToEight( const String& word )
{
    String output;
    int charnum( 0 );
    int bitpos( 0 );
    int inval;
    char outval;
    for( int i = 0; i < 8; ++i )
    {
        outval = '_';
        inval = *( (unsigned char*)( &word[charnum] ) );
        if( bitpos <= 3 ) outval += ( inval >> ( 3 - bitpos ) ) & 0x1F;
        if( bitpos > 3 ) outval += ( inval << ( bitpos - 3 ) ) & 0x1F;
        bitpos += 5;
        if( bitpos > 8 )
        {
            bitpos -= 8;
            inval = *( (unsigned char*)( &word[charnum + 1] ) );
            outval += ( inval >> ( 8 - bitpos ) );
            ++charnum;
        }
        
        if( outval != '_' )
        {
            output += outval;
        }
    }

    return output;
}

} // namespace BIP39

} // namespace Utils;

} // namespace Encryptors

} // namespace Agape
