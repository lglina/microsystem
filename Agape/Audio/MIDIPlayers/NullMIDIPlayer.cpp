#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "World/WorldCoordinates.h"
#include "NullMIDIPlayer.h"
#include "String.h"

namespace Agape
{

namespace Audio
{

namespace MIDIPlayers
{

Null::Null( AssetLoaders::Factory& assetLoaderFactory ) :
  MIDIPlayer( assetLoaderFactory )
{
}

void Null::doPlay( const World::Coordinates& coordinates, const String& assetName, bool loop )
{
}

void Null::doStop( bool hard )
{
}

void Null::programChange( int channel, int instrument )
{
}

void Null::playNote( int channel, int pitch, int velocity )
{
}

void Null::run()
{
}

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape
