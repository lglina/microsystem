#include "TelegramLoaders/FileTelegramLoader.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "FileTelegramLoaderFactory.h"
#include "String.h"

namespace Agape
{

using namespace World;

namespace TelegramLoaders
{

namespace Factories
{

File::File( const String& filename, const Metadata& metadata ) :
  m_filename( filename ),
  m_metadata( metadata )
{
}

TelegramLoader* File::makeLoader( const String& recipientSnowflake )
{
    return new TelegramLoaders::File( recipientSnowflake, m_filename, m_metadata.m_worldID );
}

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape
