#ifndef AGAPE_AUDIO_MIDI_PLAYER_H
#define AGAPE_AUDIO_MIDI_PLAYER_H

#include "Runnable.h"
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

class MIDIPlayer : public Runnable
{
public:
    MIDIPlayer( AssetLoaders::Factory& assetLoaderFactory );
    virtual ~MIDIPlayer() {};

    virtual void doPlay( const World::Coordinates& coordinates, const String& assetName, bool loop = false ) = 0;
    virtual void doStop( bool hard = false ) = 0;

    virtual void programChange( int channel, int instrument ) = 0;
    virtual void playNote( int channel, int pitch, int velocity ) = 0;

    virtual void run() = 0;

protected:
    enum State
    {
        stopped,
        playing
    };

    AssetLoaders::Factory& m_assetLoaderFactory;

    enum State m_state;
    bool m_loop;
    int m_loopNumber;
};

} // namespace Audio

} // namespace Agape

#endif // AGAPE_AUDIO_MIDI_PLAYER_H
