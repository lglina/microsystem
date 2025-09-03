#include "EntropySources/EntropySource.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "AESBlockEncryptor.h"
#include "RandomReadableWritable.h"
#include "String.h"

#include "Encryptors/AES/tiny-AES-c/aes.hpp"

#include <cstdint>

#include <string.h>

namespace
{
    const int bufferSize( AES_BLOCKLEN * 16 );
} // Anonymous namespace

namespace Agape
{

namespace Encryptors
{

AESBlock::AESBlock( EntropySource& entropySource ) :
  m_entropySource( entropySource ),
  m_writeBufferFill( 0 ),
  m_writeBufferOffset( 0 )
{
    // If we stuff up and forget to setKey(), use some random
    // key, rather than encrypting with all zeroes or something!
    m_entropySource.generate( (char*)m_key, AES_KEYLEN );

    m_writeBuffer = new char[bufferSize];
}

AESBlock::~AESBlock()
{
    delete[]( m_writeBuffer );
}

void AESBlock::setKey( const char* key )
{
#ifdef LOG_AESBLOCK
    LOG_DEBUG( "AESBlock: Key set" );
#endif
    ::memcpy( m_key, key, AES_KEYLEN );
}

bool AESBlock::open( enum OpenMode openMode, RandomReadableWritable* backingDevice )
{
    m_openMode = openMode;
    m_backingDevice = backingDevice;

    bool success( true );

    if( openMode == modeRead )
    {
        // Read IV.
        success = ( m_backingDevice->read( (char*)m_iv, 0, AES_BLOCKLEN ) == AES_BLOCKLEN );

#ifdef LOG_AESBLOCK
        LOG_DEBUG( "AESBlock: Open for reading. IV:" );
        hexDump( (char*)m_iv, AES_BLOCKLEN );
#endif
    }
    else if( openMode == modeWrite )
    {
        // Generate IV.
        m_entropySource.generate( (char*)m_iv, AES_BLOCKLEN );

        // Write IV.
        success = ( m_backingDevice->write( (char*)m_iv, 0, AES_BLOCKLEN ) == AES_BLOCKLEN );

#ifdef LOG_AESBLOCK
        LOG_DEBUG( "AESBlock: Open for writing. IV:" );
        hexDump( (char*)m_iv, AES_BLOCKLEN );
#endif
    }

    return success;
}

int AESBlock::read( char* data, int offset, int len )
{
    if( m_openMode != modeRead )
    {
        return 0;
    }

    struct AES_ctx context;

    char readBuffer[bufferSize];
    int readOffset( offset );
    int bytesFromBlockStart( readOffset % AES_BLOCKLEN );

#ifdef LOG_AESBLOCK
    {
    LiteStream stream;
    stream << "AESBlock: Read offset " << readOffset << " len " << len;
    LOG_DEBUG( stream.str() );
    }
#endif

    if( bytesFromBlockStart != 0 )
    {
        // Start back at nearest previous block boundary.
        readOffset -= bytesFromBlockStart;
#ifdef LOG_AESBLOCK
        {
        LiteStream stream;
        stream << "AESBlock: Revised offset " << readOffset;
        LOG_DEBUG( stream.str() );
        }
#endif
    }

    int lenToRead( ( len + bytesFromBlockStart ) <= bufferSize ? ( len + bytesFromBlockStart ) : bufferSize );

    initForBlock( context, readOffset );

#ifdef LOG_AESBLOCK
    {
    LiteStream stream;
    stream << "AESBlock: Reading " << lenToRead << " bytes from backing device absolute offset " << readOffset + AES_BLOCKLEN;
    LOG_DEBUG( stream.str() );
    }
#endif

    int bytesRead( m_backingDevice->read( readBuffer, readOffset + AES_BLOCKLEN, lenToRead ) ); // Add blocklen to offset as IV is stored at zero offset.

    if( bytesRead > 0 )
    {
#ifdef LOG_AESBLOCK
        {
        LiteStream stream;
        stream << "AESBlock: Read " << bytesRead << " bytes. Returning " << ( bytesRead - bytesFromBlockStart ) << " bytes from write buffer offset " << bytesFromBlockStart;
        LOG_DEBUG( stream.str() );
        }
#endif

        AES_CTR_xcrypt_buffer( &context, (std::uint8_t*)readBuffer, bytesRead );
        ::memcpy( data, ( readBuffer + bytesFromBlockStart ), ( bytesRead - bytesFromBlockStart ) );
        return ( bytesRead - bytesFromBlockStart );
    }

    return 0;
}

int AESBlock::write( const char* data, int offset, int len )
{
    if( m_openMode != modeWrite )
    {
        return 0;
    }

#ifdef LOG_AESBLOCK
    {
    LiteStream stream;
    stream << "AESBlock: Write offset " << offset << " len " << len;
    LOG_DEBUG( stream.str() );
    }
#endif

    int bytesWritten( 0 );
    while( bytesWritten < len )
    {
        int totalBytesRemain( len - bytesWritten );
        int writeBufferRemain( bufferSize - m_writeBufferFill );
        int bytesToWrite( ( totalBytesRemain <= writeBufferRemain ) ? totalBytesRemain : writeBufferRemain );
        ::memcpy( m_writeBuffer + m_writeBufferFill, data + bytesWritten, bytesToWrite );

#ifdef LOG_AESBLOCK
        {
        LiteStream stream;
        stream << "AESBlock: Write total remain " << totalBytesRemain << " write buffer fill " << m_writeBufferFill << " write buffer remain " << writeBufferRemain << " bytes to write to buffer " << bytesToWrite;
        LOG_DEBUG( stream.str() );
        }
#endif

        bytesWritten += bytesToWrite;
        m_writeBufferFill += bytesToWrite;

#ifdef LOG_AESBLOCK
        {
        LiteStream stream;
        stream << "AESBlock: Bytes written now " << bytesWritten << " write buffer fill " << m_writeBufferFill;
        LOG_DEBUG( stream.str() );
        }
#endif

        if( m_writeBufferFill == bufferSize )
        {
#ifdef LOG_AESBLOCK
            LOG_DEBUG( "AESBlock: Flush due to write buffer full." );
#endif
            flushWriteBuffer();
        }
    }

    return bytesWritten;
}

bool AESBlock::error()
{
    return false; //??
}

void AESBlock::close()
{
    if( m_openMode == modeWrite )
    {
#ifdef LOG_AESBLOCK
        LOG_DEBUG( "AESBlock: Closing" );
#endif
        flushWriteBuffer();
        m_writeBufferOffset = 0;
        m_openMode = modeNone;
    }
}

int AESBlock::overhead()
{
    return AES_BLOCKLEN;
}

void AESBlock::calculateIV( int offset, std::uint8_t* blockiv )
{
    int addend( offset / AES_BLOCKLEN );
    int carry( 0 );

#ifdef LOG_AESBLOCK
    {
    LiteStream stream;
    stream << "AESBlock: Calculating IV for offset " << offset << " addend " << addend << " base IV ";
    LOG_DEBUG( stream.str() );
    hexDump( (char*)m_iv, AES_BLOCKLEN );
    }
#endif

    ::memcpy( blockiv, m_iv, AES_BLOCKLEN );

    int byteidx( AES_BLOCKLEN - 1 );
    while( ( addend > 0 ) || ( carry > 0 ) )
    {
        int newbyte( blockiv[byteidx] + ( addend & 0xFF ) + carry );
        blockiv[byteidx] = newbyte % 0x100;

#ifdef LOG_AESBLOCK
        {
        LiteStream stream;
        stream << "AESBlock: Byte " << byteidx << "=" << m_iv[byteidx] << " addend byte " << ( addend & 0xFF ) << " carry " << carry << " blockiv byte " << blockiv[byteidx];
        LOG_DEBUG( stream.str() );
        }
#endif

        carry = newbyte / 0x100;
        addend >>= 8; // Arithmetic shift shouldn't be a problem, as addend should be positive!
        --byteidx;

        if( byteidx < ( AES_BLOCKLEN - 4 ) )
        {
            // Counter part of IV overflows at 32b boundary (per RFC3686).
            // tiny-aes-c has been modified to implement the same behaviour.
            // Note, however, that unlike per that RFC we start with a totally
            // random IV, and therefore random counter.
            break;
        }
    }

#ifdef LOG_AESBLOCK
    {
    LOG_DEBUG( "AESBlock: Final block IV:" );
    hexDump( (char*)blockiv, AES_BLOCKLEN );
    }
#endif
}

void AESBlock::initForBlock( struct AES_ctx& context, int offset )
{
    std::uint8_t blockiv[AES_BLOCKLEN];
    calculateIV( offset, blockiv );

    AES_init_ctx_iv( &context, m_key, blockiv );
}

void AESBlock::flushWriteBuffer()
{
#ifdef LOG_AESBLOCK
    LOG_DEBUG( "AESBlock: Flush called." );
#endif

    struct AES_ctx context;
    initForBlock( context, m_writeBufferOffset );
    AES_CTR_xcrypt_buffer( &context, (std::uint8_t*)m_writeBuffer, m_writeBufferFill );

#ifdef LOG_AESBLOCK
    {
    LiteStream stream;
    stream << "AESBlock: Flushing write buffer. Encrypted " << m_writeBufferFill << " backing device absolute offset " << m_writeBufferOffset + AES_BLOCKLEN;
    LOG_DEBUG( stream.str() );
    }
#endif

    m_backingDevice->write( m_writeBuffer, m_writeBufferOffset + AES_BLOCKLEN, m_writeBufferFill ); // Add blocklen to offset as IV is stored at zero offset.
    m_writeBufferOffset += m_writeBufferFill;
    m_writeBufferFill = 0;
}

} // namespace Encryptors

} // namespace Agape
