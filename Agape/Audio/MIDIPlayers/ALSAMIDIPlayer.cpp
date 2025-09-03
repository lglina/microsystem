#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Audio/FrobMIDIPlayer.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "ALSAMIDIPlayer.h"

#include <alsa/asoundlib.h>

#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <math.h>

#include "ANSITerminal.h" // Debug.
#include <sstream>

namespace Agape
{

namespace Audio
{

namespace MIDIPlayers
{

ALSA::ALSA( AssetLoaders::Factory& assetLoaderFactory ) :
  FrobMIDIPlayer( assetLoaderFactory ),
  m_seqHandle( nullptr ),
  m_seqClientID( -1 ),
  m_seqClientPort( -1 ),
  m_seqQueueHandle( -1 ),
  m_alsaQueueState( aqFreed ),
  m_stopping( false ),
  m_havePrefix( false )
{
    ::memset( m_currentPrefix, 0, 4 );
    ::memset( m_currentEvent, 0, 64 );

    m_seqReady = ( seqOpen() &&
                   seqSetName() &&
                   seqGetID() &&
                   seqCreatePort() &&
                   seqCreateSubscription() &&
                   seqSetClientPool() );
}

ALSA::~ALSA()
{
    stopPlayingQueued();

    ::snd_seq_close( m_seqHandle );
    ::snd_config_update_free_global();
}

void ALSA::playQueued()
{
    if( m_alsaQueueState != aqCreated )
    {
        std::cout << "Creating ALSA sequencer queue" << std::endl;

        if( seqCreateQueue() )
        {
            ::snd_seq_start_queue( m_seqHandle, m_seqQueueHandle, NULL );
            m_alsaQueueState = aqCreated;

            m_stopping = false;

            m_thread.reset( new std::thread( std::bind( &ALSA::playLoop, this ) ) );
        }
    }

    //std::cout << "NOTIFY FROBS QUEUED START" << std::endl;
    m_frobsQueued.notify_all();
    //std::cout << "NOTIFY FROBS QUEUED END" << std::endl;
}

void ALSA::stopPlayingQueued()
{
    if( m_alsaQueueState == aqCreated )
    {
        // Wake up and stop the play thread.
        std::cout << "Stopping play thread" << std::endl;

        std::unique_lock< std::mutex > lock( m_queueMutex );
        m_stopping = true;
        lock.unlock();
        m_frobsQueued.notify_all();
        m_thread->join();

        std::cout << "Stopped. Resetting device and freeing queue." << std::endl;

        // Clear buffer and pools.
        ::snd_seq_drop_output( m_seqHandle );

        // Stop queue (although it should now be empty).
        snd_seq_stop_queue( m_seqHandle, m_seqQueueHandle, NULL );

        // Free queue.
        seqFreeQueue();
        m_alsaQueueState = aqFreed;
        
        // Send GM reset message immediately.
        // NB: Seems to break ALSA, but works for Dream chip...
        /*
        ::snd_seq_event_t ev;
        ::snd_seq_ev_clear( &ev );
        snd_seq_ev_set_source( &ev, m_seqClientPort );
        snd_seq_ev_set_direct( &ev );
        unsigned char sysex[] = { 0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7 };
        snd_seq_ev_set_variable( &ev, 6, sysex );
        */

        // Send MIDI reset event immediately.
        std::cout << "Sending MIDI reset event" << std::endl;
        ::snd_seq_event_t ev;
        ::snd_seq_ev_clear( &ev );
        snd_seq_ev_set_source( &ev, m_seqClientPort );
        snd_seq_ev_set_direct( &ev );
        ev.type = SND_SEQ_EVENT_RESET;
        snd_seq_ev_set_fixed(&ev);
        ::snd_seq_event_output( m_seqHandle, &ev );
        ::snd_seq_drain_output( m_seqHandle );
        ::snd_seq_sync_output_queue( m_seqHandle );
        std::cout << "MIDI reset sent" << std::endl;
    }
}

bool ALSA::seqOpen()
{
    int err( ::snd_seq_open( &m_seqHandle, "default", SND_SEQ_OPEN_OUTPUT, 0 ) );
    if( err < 0 )
    {
        std::cout << "Failed to open ALSA sequencer" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqSetName()
{
    int err( ::snd_seq_set_client_name( m_seqHandle, "Agape" ) );
    if( err < 0 )
    {
        std::cout << "Failed to set client name" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqGetID()
{
    m_seqClientID = ::snd_seq_client_id( m_seqHandle );
    if( m_seqClientID < 0 )
    {
        std::cout << "Failed to get client ID" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqCreatePort()
{
    m_seqClientPort = ::snd_seq_create_simple_port( m_seqHandle,
                                                    "Agape MIDI Player",
                                                    SND_SEQ_PORT_CAP_READ,
                                                    SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION );
    if( m_seqClientPort < 0 )
    {
        std::cout << "Failed to create port" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqCreateSubscription()
{
    if( ::snd_seq_connect_to( m_seqHandle, m_seqClientPort, 128, 0 ) < 0 )
    {
        std::cout << "Failed to create subscription" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqSetClientPool()
{
    // Use relatively small client pool output size and "room" size, so that
    // snd_seq_drain_output() blocks for less time in the play thread and
    // stopping can be more responsive.
    if( ::snd_seq_set_client_pool_output_room( m_seqHandle, 32 ) < 0 )
    {
        std::cout << "Failed to set client pool output room size" << std::endl;
        return false;
    }

    if( ::snd_seq_set_client_pool_output( m_seqHandle, 64 ) < 0 )
    {
        std::cout << "Failed to set client pool output size" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqCreateQueue()
{
    m_seqQueueHandle = ::snd_seq_alloc_named_queue( m_seqHandle, "MIDI Playback" );
    if( m_seqQueueHandle < 0 )
    {
        std::cout << "Failed to create queue" << std::endl;
        return false;
    }

    return true;
}

bool ALSA::seqFreeQueue()
{
    if( ::snd_seq_free_queue( m_seqHandle, m_seqQueueHandle ) < 0 )
    {
        std::cout << "Failed to free queue" << std::endl;
        return false;
    }

    return true;
}

void ALSA::seqQueueEvent()
{
    // Queue up a MIDI event at an absolute queue time.
    int frobLength( *( (unsigned char*)&m_currentPrefix[0] ) );

    int frobTime( 0 );
    frobTime = *(unsigned char*)(&m_currentPrefix[1]) << 16;
    frobTime += *(unsigned char*)(&m_currentPrefix[2]) << 8;
    frobTime += *(unsigned char*)(&m_currentPrefix[3]);

    ::snd_seq_event_t ev;
    ::snd_seq_ev_clear( &ev );
    snd_seq_ev_set_source( &ev, m_seqClientPort );
    snd_seq_ev_set_subs( &ev );

    ::snd_seq_real_time_t rt;
    double fracSecs, intSecs;
    fracSecs = ::modf( ( (double)frobTime * 0.000256 ), &intSecs );
    rt.tv_sec = intSecs;
    rt.tv_nsec = fracSecs * 1000000000.0;
    snd_seq_ev_schedule_real( &ev, m_seqQueueHandle, 0, &rt );

    //std::cout << "Event at " << intSecs << ":" << fracSecs << " len " << frobLength << " - ";

    unsigned char val1;
    unsigned char val2;

    switch( *( (unsigned char*)&m_currentEvent[0] ) >> 4 )
    {
    case 0x8:
        // Note off
        //std::cout << "Note off";
        assert( frobLength == 3 );
        ev.type = SND_SEQ_EVENT_NOTEOFF;
        snd_seq_ev_set_fixed(&ev);
        ev.data.note.channel = m_currentEvent[0] & 0x0f;
        ev.data.note.note = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        ev.data.note.velocity = *( (unsigned char*)&m_currentEvent[2] ) & 0x7f;
        break;
    case 0x9:
        // Note on
        //std::cout << "Note on";
        assert( frobLength == 3 );
        ev.type = SND_SEQ_EVENT_NOTEON;
        snd_seq_ev_set_fixed(&ev);
        ev.data.note.channel = m_currentEvent[0] & 0x0f;
        ev.data.note.note = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        ev.data.note.velocity = *( (unsigned char*)&m_currentEvent[2] ) & 0x7f;
        break;
    case 0xa:
        // Keypress
        //std::cout << "Keypress";
        assert( frobLength == 3 );
        ev.type = SND_SEQ_EVENT_KEYPRESS;
        snd_seq_ev_set_fixed(&ev);
        ev.data.note.channel = m_currentEvent[0] & 0x0f;
        ev.data.note.note = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        ev.data.note.velocity = *( (unsigned char*)&m_currentEvent[2] ) & 0x7f;
        break;
    case 0xb:
        // Controller
        //std::cout << "Controller";
        assert( frobLength == 3 );
        ev.type = SND_SEQ_EVENT_CONTROLLER;
        snd_seq_ev_set_fixed(&ev);
        ev.data.control.channel = m_currentEvent[0] & 0x0f;
        ev.data.control.param = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        ev.data.control.value = *( (unsigned char*)&m_currentEvent[2] ) & 0x7f;
        break;
    case 0xc:
        // Program change
        //std::cout << "Program change";
        assert( frobLength == 2 );
        ev.type = SND_SEQ_EVENT_PGMCHANGE;
        snd_seq_ev_set_fixed(&ev);
        ev.data.control.channel = m_currentEvent[0] & 0x0f;
        ev.data.control.value = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        break;
    case 0xd:
        // Channel press
        //std::cout << "Channel press";
        assert( frobLength == 2 );
        ev.type = SND_SEQ_EVENT_CHANPRESS;
        snd_seq_ev_set_fixed(&ev);
        ev.data.control.channel = m_currentEvent[0] & 0x0f;
        ev.data.control.value = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        break;
    case 0xe:
        // Pitch bend
        //std::cout << "Pitch bend";
        assert( frobLength == 3 );
        ev.type = SND_SEQ_EVENT_PITCHBEND;
        snd_seq_ev_set_fixed(&ev);
        ev.data.control.channel = m_currentEvent[0] & 0x0f;
        val1 = *( (unsigned char*)&m_currentEvent[1] ) & 0x7f;
        val2 = *( (unsigned char*)&m_currentEvent[2] ) & 0x7f;
        ev.data.control.value = ( val1 | ( val2 << 7 ) ) - 0x2000;
        break;
    case 0xf:
        // SYSEX. Assume we never see meta events
        // (they're filtered out by MIDIConverter).
        // We don't handle big SYSEX (>64B) and continuations (0xF7).
        //std::cout << "SYSEX";
        snd_seq_ev_set_variable( &ev, frobLength - 1, &m_currentEvent[1] );
        break;
    default:
        std::cout << "Invalid event!";
        break;
    }

    //std::cout << std::endl;

    //std::cout << "ALSA WRITE START" << std::endl;
    ::snd_seq_event_output( m_seqHandle, &ev );
    
    // Drain every event immediately to the sequencer queue, which we've set
    // to a small size to prevent long blocking to ensure stop responsiveness.
    ::snd_seq_drain_output( m_seqHandle );
    //std::cout << "ALSA WRITE END" << std::endl;
}

void ALSA::playLoop()
{
    std::unique_lock< std::mutex > lock( m_queueMutex );
    while( !m_stopping )
    {
        if( !m_havePrefix ) // Try to get next prefix.
        {
            getPrefix();
        }

        while( m_havePrefix )
        {
            int frobLength( *( (unsigned char*)&m_currentPrefix[0] ) );

            /*
            {
            LiteStream stream;
            stream << "Have prefix for frob with length " << frobLength;
            LOG_DEBUG( stream.str() );
            }
            */

            if( m_frobBuffer.size() >= frobLength ) // We have a complete frob.
            {
                for( int i = 0; i < frobLength; ++i )
                {
                    // MIDIConverter should filter out all SYSEX greater
                    // than 64 bytes, but check here anyway to avoid
                    // buffer overflow.
                    if( i >= 64 )
                    {
                        m_frobBuffer.pop();
                        continue;
                    }

                    m_currentEvent[i] = m_frobBuffer.pop();
                }

                //LOG_DEBUG( "Frob complete. Data: " );
                //hexDump( m_currentEvent, frobLength );

                /*
                std::ostringstream oss;
                oss << ANSITerminal::colour( ANSITerminal::colBrightGreen );
                char debugBuffer[64];
                debugBuffer[0] = m_currentPrefix[0];
                debugBuffer[1] = m_currentPrefix[1];
                debugBuffer[2] = m_currentPrefix[2];
                debugBuffer[3] = m_currentPrefix[3];
                for( int i = 0; i < frobLength; ++i )
                {
                    debugBuffer[4+i] = m_currentEvent[i];
                }
                oss << toHexStr( debugBuffer, frobLength + 4 );
                oss << ANSITerminal::reset();
                std::cout << oss.str();
                */

                lock.unlock(); // Don't block producer if we're blocked on ALSA.
                seqQueueEvent();
                lock.lock();

                getPrefix();
            }
        }

        if( !m_stopping )
        {
            //LOG_DEBUG( "Waiting for more frobs" );
            m_frobsQueued.wait( lock );
        }
    }
}

void ALSA::getPrefix()
{
    if( m_frobBuffer.size() > 4 ) // We have a prefix (time and size).
    {
        m_currentPrefix[0] = m_frobBuffer.pop();
        m_currentPrefix[1] = m_frobBuffer.pop();
        m_currentPrefix[2] = m_frobBuffer.pop();
        m_currentPrefix[3] = m_frobBuffer.pop();
        m_havePrefix = true;

        //LOG_DEBUG( "ALSAMIDIPlayer: Have prefix:" );
        //hexDump( m_currentPrefix, 3 );
    }
    else
    {
        m_havePrefix = false;
    }
}

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape
