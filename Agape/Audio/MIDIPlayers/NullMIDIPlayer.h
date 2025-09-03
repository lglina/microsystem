#ifndef AGAPE_AUDIO_MIDI_PLAYERS_NULL_H
#define AGAPE_AUDIO_MIDI_PLAYERS_NULL_H

#include "Audio/MIDIPlayer.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{
    class Factory;
}

namespace Audio
{

namespace MIDIPlayers
{

class Null : public MIDIPlayer
{
public:
    Null( AssetLoaders::Factory& assetLoaderFactory );

    virtual void doPlay( const World::Coordinates& coordinates, const String& assetName, bool loop = false );
    virtual void doStop( bool hard = false );

    virtual void programChange( int channel, int instrument );
    virtual void playNote( int channel, int pitch, int velocity );

    virtual void run();
};

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape

#endif // AGAPE_AUDIO_MIDI_PLAYERS_NULL_H
