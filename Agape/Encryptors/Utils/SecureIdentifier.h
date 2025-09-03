#ifndef AGAPE_ENCRYPTORS_UTILS_SECURE_IDENTIFIER_H
#define AGAPE_ENCRYPTORS_UTILS_SECURE_IDENTIFIER_H

#include "String.h"

namespace Agape
{

class Encryptor;
class Hash;

namespace Encryptors
{

namespace Utils
{

/// The secure identifier mechanism creates a unique, encrypted name for an
/// object consisting of both a hashed and encrypted version of that name.
///
/// Secure identifier strings consist of two parts:
/// * 32B SHA256 hash of concatenated encryption key and object name
/// * 16B AES IV
/// * Remaining bytes AES-encrypted object name
///
/// The name generated satisfies the following properties:
/// * Anyone receiving the name, and having the correct key, can decrypt it.
/// * The party encrypting a name can select a random IV for AES encryption
///   (with the IV needed for decryption being stored concatenated with
///   the ciphertext)
/// * Any party wanting to request an object by name, despite not knowing the
///   AES IV (and therefore not being able to generate a matching ciphertext
///   a-priori) can still request the object using its hashed name (which anyone
///   can generate if they have the correct key and asset name).
/// * The key is concatenated with the object name prior to hashing, such that
///   no two plaintexts will hash to the same value if different keys are used.
///
/// Although an identifier is a single string, because the hash is fixed-length,
/// it is possible to store each identifier as two separate parts (hash and
/// encrypted envelope), to allow indexing of the hash part for rapid
/// object look-up.
class SecureIdentifier
{
public:
    static String createIdentifier( Encryptor& encryptor,
                                    Hash& hash,
                                    const char* key,
                                    const String& objectNamePlain );
    static void splitIdentifier( const String& identifier,
                                 String& hash,
                                 String& cipherText );
};

} // namespace Utils

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_UTILS_SECURE_IDENTIFIER_H
