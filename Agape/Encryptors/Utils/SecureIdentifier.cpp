#include "Encryptors/Encryptor.h"
#include "Encryptors/Hash.h"
#include "Utils/base64/base64.h"
#include "SecureIdentifier.h"
#include "String.h"

namespace Agape
{

namespace Encryptors
{

namespace Utils
{

String SecureIdentifier::createIdentifier( Encryptor& encryptor,
                                           Hash& hash,
                                           const char* key,
                                           const String& objectNamePlain )
{
    // Hash (key || objectName).
    hash.reset();
    String input( String( key, encryptor.keySize() ) + objectNamePlain );
    hash.update( &input[0], input.length() );
    String hashedObjectName( hash.digestSize(), '\0' );
    hash.finalise( &hashedObjectName[0] );

    // Encrypt object name and store as envelope (IV || ciphertext).
    encryptor.setKey( key );
    String encryptedObjectName( encryptor.encrypt( objectNamePlain, false ) ); // false = no Base-64.

    // Base-64 encode (hash || envelope).
    String name = ( hashedObjectName + encryptedObjectName );
    String encodedName( Base64encode_len( name.size() ), '\0' );
    Base64encode( &encodedName[0],
                  &name[0],
                  name.size() );
    encodedName.resize( encodedName.length() - 1 );

    return encodedName;
}

void SecureIdentifier::splitIdentifier( const String& identifier,
                                        String& hash,
                                        String& cipherText )
{
    // Base-64 decode identifier.
    String decodedIdentifier;
    decodedIdentifier.resize( Base64decode_len( identifier.c_str() ), '\0' );
    decodedIdentifier.resize( Base64decode( &decodedIdentifier[0], identifier.c_str() ) );

    if( decodedIdentifier.length() < 48 ) return; // Expect at least 32B hash, 16B IV.

    // Extract hash and re-encode to Base-64
    String decodedHash( decodedIdentifier.substr( 0, 32 ) );
    hash.resize( Base64encode_len( decodedHash.length() ), '\0' );
    Base64encode( &hash[0], &decodedHash[0], decodedHash.length() );
    hash.resize( hash.length() - 1 );

    // Extract encrypted envelope and re-encode to Base-64
    String decodedEnvelope( decodedIdentifier.substr( 32 ) );
    cipherText.resize( Base64encode_len( decodedEnvelope.length() ), '\0' );
    Base64encode( &cipherText[0], &decodedEnvelope[0], decodedEnvelope.length() );
    cipherText.resize( cipherText.length() - 1 );
}

} // namespace Utils

} // namespace Encryptors

} // namespace Agape
