#include "Assets/Asset.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "FrobMIDIPlayer.h"
#include "MIDIPlayer.h"

#ifndef __XC32
#include "ANSITerminal.h" // Debugging
#include <iostream>
#include <sstream>
#endif

namespace
{
    const int frobBufferSize( 2048 );
    const int frobMaxLength( 64 );
} // Anonymous namespace

namespace Agape
{

namespace Audio
{

FrobMIDIPlayer::FrobMIDIPlayer( AssetLoaders::Factory& assetLoaderFactory ) :
  MIDIPlayer( assetLoaderFactory ),
  m_frobBuffer( frobBufferSize ),
  m_assetLoader( nullptr ),
  m_asset( nullptr ),
  m_assetOffset( 0 ),
  m_lastFrobTime( 0 ),
  m_timeOffset( 0 )
{
}

FrobMIDIPlayer::~FrobMIDIPlayer()
{
    // Soft stop only here as we cannot call the derived class
    // stopPlayingQueued(). The derived class destructor should call that
    // function itself.
    doStop( false );
    delete m_asset;
    delete m_assetLoader;
}

void FrobMIDIPlayer::doPlay( const World::Coordinates& coordinates, const String& assetName, bool loop )
{
    LOG_DEBUG( "MIDIPlayer: Play." );

    doStop( true ); // true == hard stop.

    m_assetLoader = m_assetLoaderFactory.makeLoader( coordinates, assetName );

    if( m_assetLoader->open() )
    {
        m_asset = new Asset( *m_assetLoader );
        m_state = playing;
        m_loop = loop;
        m_loopNumber = 0;
        LOG_DEBUG( "MIDIPlayer: Asset opened successfully." );
    }
    else
    {
        delete( m_assetLoader );
        m_assetLoader = nullptr;
        m_state = stopped;
        LOG_DEBUG( "MIDIPlayer: Failed to open asset " + assetName );
    }
}

void FrobMIDIPlayer::doStop( bool hard )
{
    if( hard )
    {
        LOG_DEBUG( "MIDIPlayer: Hard stop." );
    }
    else
    {
        LOG_DEBUG( "MIDIPlayer: Soft stop." );
    }

    delete( m_asset );
    m_asset = nullptr;
    
    if( m_assetLoader ) m_assetLoader->close();
    delete( m_assetLoader );
    m_assetLoader = nullptr;

    // A hard stop terminates playing of any queued frobs immediately and
    // purges the buffer. Soft stop leaves the remaining buffered events alone
    // and the sub-class implementation can keep emitting them at the
    // appropriate times.
    if( hard )
    {
        // If the sub-class uses a separate playing thread or interrupt, allow the
        // sub-class to politely signal that to stop before we rudely clear the
        // frob queue (perhaps while the thread is half-way through reading a frob).
        stopPlayingQueued();

        m_frobBuffer.clear();
    }

    m_state = stopped;

    m_loop = false;
    m_loopNumber = 0;
    
    m_assetOffset = 0;
    m_lastFrobTime = 0;
    m_timeOffset = 0;
}

void FrobMIDIPlayer::programChange( int channel, int instrument )
{
    // FIXME: Stub.
}

void FrobMIDIPlayer::playNote( int channel, int pitch, int velocity )
{
    // FIXME: Stub.
}

void FrobMIDIPlayer::run()
{
    if( m_state == playing )
    {
        char assetBuffer[128];
        bool didQueue( false );
        while( ( m_state == playing ) &&
               ( m_frobBuffer.free() >= frobMaxLength ) )
        {
            {
            LiteStream stream;
            stream << "MIDIPlayer: Buffer size: " << m_frobBuffer.size();
            LOG_DEBUG( stream.str() );
            }
            LOG_DEBUG( "MIDIPlayer: Will enqueue." );

            int assetNumRead( m_asset->read( assetBuffer, m_assetOffset, 128 ) );

            /*
            {
            LiteStream stream;
            stream << "MIDIPlayer: Read " << assetNumRead << " bytes from asset at offset " << m_assetOffset << ".";
            LOG_DEBUG( stream.str() );
            }
            */

            m_assetOffset += assetNumRead;

            int assetBufferOffset( 0 );
            while( 1 )
            {
                int assetBufferRemain( assetNumRead - assetBufferOffset );
                bool incomplete( true );
                if( assetBufferRemain > 3 ) // Might have complete frob?
                {
                    int frobLength( ( *( (unsigned char*)&assetBuffer[assetBufferOffset] ) & 0xF0 ) >> 4 );

                    /*
                    {
                    LiteStream stream;
                    stream << "Asset buffer pos " << assetBufferOffset << " frob len " << frobLength;
                    LOG_DEBUG( stream.str() );
                    }
                    */

                    if( assetBufferRemain >= ( frobLength + 3 ) ) // Complete frob.
                    {
                        int frobTime( 0 );
                        frobTime = ( *(unsigned char*)(&assetBuffer[assetBufferOffset++]) & 0x0F ) << 16;
                        frobTime += *(unsigned char*)(&assetBuffer[assetBufferOffset++]) << 8;
                        frobTime += *(unsigned char*)(&assetBuffer[assetBufferOffset++]);

                        m_lastFrobTime = frobTime;

                        /*
                        {
                        LiteStream stream;
                        stream << "Frob complete. Time " << frobTime << " + time offset" << m_timeOffset;
                        LOG_DEBUG( stream.str() );
                        }
                        */

                        frobTime += m_timeOffset;

                        char debugBuffer[64];

                        unsigned char c;
                        c = frobLength; m_frobBuffer.push( *( (char*)&c ) ); debugBuffer[0] = c;
                        c = ( frobTime >> 16 ) & 0xFF; m_frobBuffer.push( *( (char*)&c ) ); debugBuffer[1] = c;
                        c = ( frobTime >> 8 ) & 0xFF; m_frobBuffer.push( *( (char*)&c ) ); debugBuffer[2] = c;
                        c = frobTime & 0xFF; m_frobBuffer.push( *( (char*)&c ) ); debugBuffer[3] = c;

                        for( int i = 0; ( frobLength <= frobMaxLength ) && ( i < frobLength ); ++i )
                        {
                            char b( assetBuffer[assetBufferOffset++] );
                            m_frobBuffer.push( b );
                            debugBuffer[4+i] = b;
                        }

#ifndef __XC32
/*
                        std::ostringstream oss;
                        oss << ANSITerminal::colour( ANSITerminal::colBrightRed );
                        oss << toHexStr( debugBuffer, frobLength + 4 );
                        oss << ANSITerminal::reset();
                        std::cout << oss.str();
*/
#endif

                        didQueue = true;
                        incomplete = false;
                    }
                }
                else if( m_assetOffset == m_asset->size() ) // End of file. Loop?
                {
                    if( m_loop && ( m_loopNumber < 2 ) && ( ( m_timeOffset + m_lastFrobTime ) <= 0xFFFFFF ) ) // Don't loop to overflow of frob time.
                    {
                        {
                        LiteStream stream;
                        stream << "EOF. Adding time offset " << m_lastFrobTime;
                        LOG_DEBUG( stream.str() );
                        }

                        m_timeOffset += m_lastFrobTime;
                        m_assetOffset = 0;
                        ++m_loopNumber;
                        break; // Go back to outer loop to read file from beginning.
                    }
                    else
                    {
                        // Stop playing.
                        LOG_DEBUG( "EOF. Stopping." );
                        doStop();
                        break;
                    }
                }
                else if( assetNumRead == 0 )
                {
                    // Read error?
                    LOG_DEBUG( "Zero read. Stopping." );
                    doStop();
                    break;
                }

                if( ( m_frobBuffer.free() < frobMaxLength ) || incomplete )
                {
                    // Only a partial frob in the asset read buffer or
                    // output buffer is now almost full.
                    // Move offset to frob start and break to re-read.
                    m_assetOffset -= assetBufferRemain;

                    /*
                    {
                    LiteStream stream;
                    stream << "Incomplete frob. Setting offset to " << m_assetOffset;
                    LOG_DEBUG( stream.str() );
                    }
                    */

                    break;
                }
            }
        }
        
        /*
        if( ( m_state == playing ) &&
            ( m_frobBuffer.size() > ( frobBufferSize / 2 ) ) )
        {
            LOG_DEBUG( "MIDIPlayer: Buffer filling. Will stop enqueueing." );
        }
        */

        if( didQueue )
        {
            // Call sub-class handler to play frobs from queue or signal a
            // playing thread or whatever...
            playQueued();
        }
    }
}

} // namespace Audio

} // namespace Agape
