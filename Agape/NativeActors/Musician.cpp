#include "Audio/MIDIPlayer.h"
#include "Musician.h"
#include "String.h"
#include "Tuple.h"
#include "TupleRouter.h"

// TODO: This was created for a demo where we only needed to play a few notes
// with a few instruments. It needs to be fleshed out to create a proper
// tuple-based interface to the MIDI module.

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

Musician::Musician( TupleRouter& tupleRouter, Audio::MIDIPlayer& midiPlayer ) :
  Native( "Musician" ),
  m_tupleRouter( tupleRouter ),
  m_midiPlayer( midiPlayer ),
  m_bassNote( 0 )
{
    m_tupleRouter.registerActor( this );

    /*
    m_midiPlayer.programChange( 9, 0 ); // Drums
    m_midiPlayer.programChange( 1, 39 ); // Synth bass
    m_midiPlayer.programChange( 2, 25 ); // Guitar

    m_midiPlayer.playNote( 1, 20, 100 );
    m_midiPlayer.playNote( 2, 10, 50 );
    */
}

Musician::~Musician()
{
    m_tupleRouter.deregisterActor( this );
}

bool Musician::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::destinationActor( tuple ) == "Musician" )
    {
        String tupleType( TupleRouter::tupleType( tuple ) );
        if( tupleType == "playNote" )
        {
            int channel( 0 );
            int pitch( 0 );

            String instrument = tuple["instrument"];
            if( instrument == "drum1" )
            {
                channel = 9;
                pitch = 35;
            }
            else if( instrument == "drum2" )
            {
                channel = 9;
                pitch = 38;
            }
            else if( instrument == "bass" || instrument == "guitar" )
            {
                if( instrument == "bass" )
                {
                    channel = 1;
                }
                else if( instrument == "guitar" )
                {
                    channel = 2;
                }

                if( m_bassNote == 0 )
                {
                    pitch = 40;
                }
                else if( m_bassNote == 1 )
                {
                    pitch = 36;
                }
                else if( m_bassNote == 2 )
                {
                    pitch = 33;
                }
                else if( m_bassNote == 3 )
                {
                    pitch = 31;
                }
                else if( m_bassNote == 4 )
                {
                    pitch = 33;
                }
                else if( m_bassNote == 5 )
                {
                    pitch = 35;
                }
                else if( m_bassNote == 6 )
                {
                    pitch = 38;
                }
                else if( m_bassNote == 7 )
                {
                    pitch = 38;
                }

                ++m_bassNote;
                if( m_bassNote == 8 )
                {
                    m_bassNote = 0;
                }
            }
            else
            {
                String key = tuple["key"];
                int octave( key[1] - '0' );
                pitch = ( octave * 12 ) + 12;
                switch( key[0] )
                {
                case 'A':
                    pitch += 9;
                    break;
                case 'B':
                    pitch += 11;
                    break;
                case 'C':
                    break;
                case 'D':
                    pitch += 1;
                    break;
                case 'E':
                    pitch += 4;
                    break;
                case 'F':
                    pitch += 5;
                    break;
                case 'G':
                    pitch += 7;
                    break;
                default:
                    break;
                }
            }

            m_midiPlayer.playNote( channel, pitch, 0x3f );

            handled = true;
        }
    }

    return handled;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
