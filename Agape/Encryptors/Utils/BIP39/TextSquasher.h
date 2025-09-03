#ifndef AGAPE_ENCRYPTORS_UTILS_BIP39_TEXT_SQUASHER_H
#define AGAPE_ENCRYPTORS_UTILS_BIP39_TEXT_SQUASHER_H

#include "String.h"

namespace Agape
{

namespace Encryptors
{

namespace Utils
{

namespace BIP39
{

String squashEightToFive( const String& word );
String unsquashFiveToEight( const String& word );

} // namespace BIP39

} // namespace Utils;

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_UTILS_BIP39_TEXT_SQUASHER_H
