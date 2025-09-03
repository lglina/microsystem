#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_MUSICIAN_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_MUSICIAN_H

#include "Actors/NativeActors/NativeActor.h"

namespace Agape
{

namespace Audio
{
class MIDIPlayer;
} // namespace Audio

namespace Linda2
{

class TupleRouter;
class Tuple;

namespace Actors
{

namespace NativeActors
{

class Musician : public Actors::Native
{
public:
    Musician( TupleRouter& tupleRouter, Audio::MIDIPlayer& midiPlayer );
    virtual ~Musician();

    virtual bool accept( Tuple& tuple );

private:
    TupleRouter& m_tupleRouter;
    Audio::MIDIPlayer& m_midiPlayer;

    int m_bassNote;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_MUSICIAN_H
