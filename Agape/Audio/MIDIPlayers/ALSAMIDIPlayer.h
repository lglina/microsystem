#ifndef AGAPE_AUDIO_MIDI_PLAYERS_ALSA_H
#define AGAPE_AUDIO_MIDI_PLAYERS_ALSA_H

#include "Audio/FrobMIDIPlayer.h"
#include "String.h"

#include <alsa/asoundlib.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace Agape
{

namespace Audio
{

namespace MIDIPlayers
{

class ALSA : public FrobMIDIPlayer
{
public:
    ALSA( AssetLoaders::Factory& assetLoaderFactory );
    virtual ~ALSA();

    virtual void playQueued();
    virtual void stopPlayingQueued();

private:
    enum ALSAQueueState
    {
        aqCreated,
        aqFreed
    };

    bool seqOpen();
    bool seqSetName();
    bool seqGetID();
    bool seqCreatePort();
    bool seqCreateSubscription();
    bool seqSetClientPool();
    bool seqCreateQueue();
    bool seqFreeQueue();
    void seqQueueEvent();

    void playLoop();
    void getPrefix();

    snd_seq_t* m_seqHandle;
    int m_seqClientID;
    int m_seqClientPort;
    int m_seqQueueHandle;
    bool m_seqReady;

    enum ALSAQueueState m_alsaQueueState;

    std::unique_ptr< std::thread > m_thread;
    std::mutex m_queueMutex;
    std::condition_variable m_frobsQueued;
    bool m_stopping;

    char m_currentPrefix[4];
    char m_currentEvent[64];
    bool m_havePrefix;
};

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape

#endif // AGAPE_AUDIO_MIDI_PLAYERS_ALSA_H
