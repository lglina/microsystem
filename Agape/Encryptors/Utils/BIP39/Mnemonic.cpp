#include "Encryptors/SHA256/SHA256Hash.h"
#include "Collections.h"
#include "Mnemonic.h"
#include "MnemonicWordlist.h"
#include "String.h"
#include "TextSquasher.h"

//#include <iostream>

namespace Agape
{

namespace Encryptors
{

namespace Utils
{

namespace BIP39
{

bool Mnemonic::encode( const char* key, Vector< String >& words )
{
    // Assumes 128 bit key.
    Hashes::SHA256 sha256;
    sha256.update( key, 16 );
    char* digest = new char[sha256.digestSize()];
    sha256.finalise( digest );

    int checksum( ( digest[0] >> 4 ) & 0xF );

    unsigned char* keyptr( (unsigned char*)( key ) );
    int maskShift( 0 );
    int idx( 0 );
    for( int i = 0; i < 12; ++i )
    {
        if( maskShift == 0 ) maskShift = 8;
        idx = 0;
        int nextChar( *keyptr );
        idx += ( nextChar & ( ( 1 << maskShift ) - 1 ) ) << ( 11 - maskShift );
        if( ( keyptr - (unsigned char*)key ) == 15 ) break;
        maskShift = 8 - ( 11 - maskShift );
        ++keyptr;
        if( maskShift >= 0 )
        {
            nextChar = *keyptr;
            idx += nextChar >> maskShift;

            if( maskShift == 0 )
            {
                ++keyptr;
            }
        }
        else
        {
            nextChar = *keyptr;
            idx += nextChar << ( 0 - maskShift );
            ++keyptr;
            maskShift += 8;
            nextChar = *keyptr;
            idx += nextChar >> maskShift;
        }

        words.push_back( getWordByIndex( idx ) );
    }

    idx += checksum;
    words.push_back( getWordByIndex( idx ) );

    delete[]( digest );

    return true;
}

bool Mnemonic::decode( const Vector< String >& words, char* key )
{
    // Assumes 128 bit key.
    unsigned char* keyptr( (unsigned char*)key );
    *keyptr = 0;
    int shift( 8 );
    int idx( 0 );
    for( int i = 0; i < 12; ++i )
    {
        idx = getIndexByWord( words[i] );
        shift = 11 - shift;
        *keyptr += idx >> shift;
        if( ( keyptr - (unsigned char*)key ) == 15 ) break;
        ++keyptr;
        shift = 8 - shift;
        if( shift >= 0 )
        {
            *keyptr = idx << shift;
        }
        else
        {
            *keyptr = idx >> ( 0 - shift );
            ++keyptr;
            shift += 8;
            *keyptr = idx << shift;
        }

        if( shift == 0 )
        {
            shift = 8;
            ++keyptr;
            *keyptr = 0;
        }
    }

    Hashes::SHA256 sha256;
    sha256.update( key, 16 );
    char* digest = new char[sha256.digestSize()];
    sha256.finalise( digest );

    int checksum( ( digest[0] >> 4 ) & 0xF );

    delete[]( digest );

    //std::cout << "Checksum " << checksum << " == " << ( idx & 0x0F ) << std::endl;

    return( checksum == ( idx & 0x0F ) );
}

bool Mnemonic::completeWord( const String& stem, String& completion )
{
    int index( getIndexByWord( stem ) );
    if( index != -1 )
    {
        completion = getWordByIndex( index );
        return true;
    }

    return false;
}

String Mnemonic::getWordByIndex( int index )
{
    String encodedWord( (char*)mnemonicWordlist[index], 5 );
    return( unsquashFiveToEight( encodedWord ) );
}

int Mnemonic::getIndexByWord( const String& word )
{
    String encodedWord( squashEightToFive( word ) );

    int start( 0 );
    int end( 2047 );
    while( start <= end )
    {
        int mid( ( start + end ) / 2 );

        if( mid == 0 )
        {
            mid = 0;
        }

        if( compareSquashedStems( encodedWord, (const char*)mnemonicWordlist[mid] ) == 0 ) // Match
        {
            return mid;
        }
        else if( compareSquashedStems( encodedWord, (const char*)mnemonicWordlist[mid] ) == 1 ) // Less than
        {
            end = mid - 1;
        }
        else // Greater than
        {
            start = mid + 1;
        }
    }

    return -1;
}

inline int Mnemonic::compareSquashedStems( const String& first, const char* second )
{
    long ifirst( 0 );
    long isecond( 0 );

    const unsigned char* fptr( (const unsigned char*)&first[0] );
    const unsigned char* sptr( (const unsigned char*)second );

    ifirst += (long)fptr[0] << 12;
    ifirst += (long)fptr[1] << 4;
    ifirst += fptr[2] >> 4;

    isecond += (long)sptr[0] << 12;
    isecond += (long)sptr[1] << 4;
    isecond += sptr[2] >> 4;

    if( ifirst == isecond )
    {
        return 0;
    }
    else if( ifirst < isecond )
    {
        return 1;
    }

    return -1;
}

} // namespace BIP39

} // namespace Utils;

} // namespace Encryptors

} // namespace Agape
