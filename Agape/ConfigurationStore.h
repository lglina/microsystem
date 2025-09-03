#ifndef AGAPE_CONFIGURATION_STORE_H
#define AGAPE_CONFIGURATION_STORE_H

#include "Memories/Memory.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

class ConfigurationStore
{
public:
    ConfigurationStore( Memory& memory );

    bool hasKey( const String& key );
    Value& get( const String& key );
    void remove( const String& key );

    void save();

private:
    void load();

    Memory& m_memory;

    bool m_loaded;
    Value m_store;
};

} // namespace Agape

#endif // AGAPE_CONFIGURATION_STORE_H
