#include "Loggers/Logger.h"
#include "Memories/Memory.h"
#include "ConfigurationStore.h"
#include "String.h"
#include "Value.h"

// FIXME: Move Value out of Linda2 namespace.
using namespace Agape::Linda2;

namespace Agape
{

ConfigurationStore::ConfigurationStore( Memory& memory ) :
  m_memory( memory ),
  m_loaded( false )
{
}

bool ConfigurationStore::hasKey( const String& key )
{
    if( !m_loaded ) load();
    return m_store.hasValue( key );
}

Value& ConfigurationStore::get( const String& key )
{
    if( !m_loaded ) load();
    return m_store[key];
}

void ConfigurationStore::remove( const String& key )
{
    if( !m_loaded ) load();
    m_store.erase( key );
}

void ConfigurationStore::save()
{
    if( !m_loaded ) load();
    LOG_DEBUG( "Saving configuration store" );
    LOG_DEBUG( m_store.dump() );
    // FIXME: Only erasing one sector. How to ensure m_store.toReadabeWritable
    // doesn't overflow?
    m_memory.erase( 0, m_memory.sectorSize() );
    m_memory.seek( 0 );
    m_store.toReadableWritable( m_memory );
    m_memory.flushOutput();
}

void ConfigurationStore::load()
{
    m_memory.seek( 0 );
    Value::fromReadableWritable( m_memory, m_store );
    LOG_DEBUG( "Loading configuration store" );
    LOG_DEBUG( m_store.dump() );

    m_loaded = true;
}

} // namespace Agape
