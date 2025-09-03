#ifndef AGAPE_TELEGRAM_LOADERS_FACTORIES_FILE_H
#define AGAPE_TELEGRAM_LOADERS_FACTORIES_FILE_H

#include "String.h"
#include "TelegramLoadersFactory.h"

namespace Agape
{

namespace World
{
class Metadata;
} // namespace World

using namespace World;

class TelegramLoader;

namespace TelegramLoaders
{

namespace Factories
{

class File : public Factory
{
public:
    File( const String& filename, const Metadata& metadata );
    
    virtual TelegramLoader* makeLoader( const String& recipientSnowflake );

private:
    String m_filename;
    const Metadata& m_metadata;
};

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_FACTORIES_FILE_H
