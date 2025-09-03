#ifndef AGAPE_ENCRYPTORS_UTILS_BIP39_MNEMONIC_H
#define AGAPE_ENCRYPTORS_UTILS_BIP39_MNEMONIC_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Encryptors
{

namespace Utils
{

namespace BIP39
{

class Mnemonic
{
public:
    static bool encode( const char* key, Vector< String >& words );
    static bool decode( const Vector< String >& words, char* key );

    static bool completeWord( const String& stem, String& completion );

private:
    static String getWordByIndex( int index );
    static int getIndexByWord( const String& word );

    inline static int compareSquashedStems( const String& first, const char* second );
};

} // namespace BIP39

} // namespace Utils;

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_UTILS_BIP39_MNEMONIC_H
