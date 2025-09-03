#ifndef AGAPE_AUDIO_SAM2695_PLAYER_H
#define AGAPE_AUDIO_SAM2695_PLAYER_H

#include "Audio/FrobMIDIPlayer.h"
#include "InterruptHandler.h"

#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace World
{
class Coordinates;
} // namespace World

class PICSerial;

namespace Audio
{

namespace MIDIPlayers
{

class SAM2695 : public FrobMIDIPlayer, public InterruptHandler
{
public:
    SAM2695( AssetLoaders::Factory& assetLoaderFactory, PICSerial& midiOut );

    virtual void programChange( int channel, int instrument );
    virtual void playNote( int channel, int pitch, int velocity );

    virtual void handleInterrupt( enum InterruptDispatcher::InterruptVector vector );

private:
    virtual void playQueued();
    virtual void stopPlayingQueued();

    void getPrefix();

    PICSerial& m_midiOut;

    long m_frobClock;
    long m_nextFrobLength;
    long m_nextFrobTime;
    bool m_havePrefix;

    int m_lateFrobs;
};

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape

#endif // AGAPE_AUDIO_SAM2695_PLAYER_H
