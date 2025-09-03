#ifndef AGAPE_AUDIO_FROB_MIDI_PLAYER_H
#define AGAPE_AUDIO_FROB_MIDI_PLAYER_H

#include "Utils/RingBuffer.h"
#include "MIDIPlayer.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class Asset;
class AssetLoader;

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Audio
{

class FrobMIDIPlayer : public MIDIPlayer
{
public:
    FrobMIDIPlayer( AssetLoaders::Factory& assetLoaderFactory );
    virtual ~FrobMIDIPlayer();

    virtual void doPlay( const World::Coordinates& coordinates, const String& assetName, bool loop = false );
    virtual void doStop( bool hard = false );

    virtual void programChange( int channel, int instrument );
    virtual void playNote( int channel, int pitch, int velocity );

    virtual void run();

protected:
    // NOTE: Implementations MUST handle incomplete frobs in the buffer, e.g.
    // by checking if the buffer has > 3 bytes free, then checking the frob
    // length to see if the buffer has the complete frob.
    virtual void playQueued() = 0;
    virtual void stopPlayingQueued() = 0;

    RingBuffer< char > m_frobBuffer;

private:
    AssetLoader* m_assetLoader;
    Asset* m_asset;
    int m_assetOffset;
    int m_lastFrobTime;
    int m_timeOffset;
};

} // namespace Audio

} // namespace Agape

#endif // AGAPE_AUDIO_FROB_MIDI_PLAYER_H
