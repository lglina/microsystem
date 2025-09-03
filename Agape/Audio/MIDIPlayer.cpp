#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "MIDIPlayer.h"

namespace Agape
{

namespace Audio
{

MIDIPlayer::MIDIPlayer( AssetLoaders::Factory& assetLoaderFactory ) :
  m_assetLoaderFactory( assetLoaderFactory ),
  m_state( stopped ),
  m_loop( false ),
  m_loopNumber( 0 )
{
}

} // namespace Audio

} // namespace Agape
