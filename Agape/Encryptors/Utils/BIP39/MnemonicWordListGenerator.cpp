#ifdef BIP39_GENERATOR

#include "Utils/StrToHex.h"
#include "String.h"
#include "TextSquasher.h"

#include <fstream>
#include <iostream>

using namespace Agape;
using namespace Agape::Encryptors::Utils::BIP39;

int main( int argc, char** argv )
{
    std::ifstream wordlist( "wordlist.txt" );
    if( wordlist.is_open() )
    {
        String line;
        std::cout << "const char[2048][5] {" << std::endl;
        while( getline( wordlist, line ) )
        {
            String encoded( squashEightToFive( line ) );
            //String decoded( unsquashFiveToEight( encoded ) );
            //std::cout << line << std::endl;
            //std::cout << decoded << std::endl;
            //std::cout << strToHex( line ) << std::endl;
            //std::cout << strToHex( encoded ) << std::endl;
            //std::cout << strToHex( decoded ) << std::endl;

            std::cout << "{ ";
            for( int i = 0; i < 5; ++i )
            {
                std::cout << "0x" << strToHex( String( 1, encoded[i] ) );
                if( i != 4 )
                {
                    std::cout << ", ";
                }
            }
            std::cout << "}," << std::endl;
           //break;
        }
        std::cout << "};" << std::endl;
    }
}

#endif // BIP39_GENERATOR
