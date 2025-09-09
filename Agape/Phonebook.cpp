#include "Collections.h"
#include "ConfigurationStore.h"
#include "Phonebook.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

using Agape::String;

namespace Agape
{

Phonebook::Phonebook( ConfigurationStore& configurationStore ) :
  m_configurationStore( configurationStore )
{
}

void Phonebook::add( const String& name, const String& number, bool setDefault )
{
    Value& phonebook( m_configurationStore.get( _phonebook ) );
    Value entry;
    entry[_number] = number;
    // FIXME: Should this be hard-coded, or a user option?
    entry[_authenticate] = ( ( number.find( "glina.com.au" ) != String::npos ) ||
                             ( number.find( "localhost" ) != String::npos ) ||
                             ( number.find( "127.0.0.1" ) != String::npos ) )
                           ? 1 : 0;
    phonebook[name] = entry;

    if( setDefault )
    {
        setDefaultEntry( name );
    }

    m_configurationStore.save();
}

void Phonebook::remove( const String& name )
{
    if( hasDefaultEntry() )
    {
        String defaultName;
        String defaultNumber;
        getDefaultEntry( defaultName, defaultNumber );
        if( defaultName == name )
        {
            m_configurationStore.remove( _defaultPhonebookEntry );
        }
    }

    Value& phonebook( m_configurationStore.get( _phonebook ) );
    phonebook.erase( name );

    m_configurationStore.save();
}

Map< String, String > Phonebook::getEntries() const
{
    Map< String, String > entries;
    Value& phonebook( m_configurationStore.get( _phonebook ) );
    ConstMapIterator it( phonebook.mapBegin() );
    for( ; it != phonebook.mapEnd(); ++it )
    {
        String currentName( it->first );
        Value* currentEntry( it->second );
        entries[currentName] = ( *currentEntry )[_number];
    }

    return entries;
}

bool Phonebook::hasDefaultEntry() const
{
    return( m_configurationStore.hasKey( _phonebook ) && m_configurationStore.hasKey( _defaultPhonebookEntry ) );
}

bool Phonebook::getDefaultEntry( String& name, String& number ) const
{
    if( m_configurationStore.hasKey( _phonebook ) && m_configurationStore.hasKey( _defaultPhonebookEntry ) )
    {
        Value& phonebook( m_configurationStore.get( _phonebook ) );
        String defaultPhonebookEntry = m_configurationStore.get( _defaultPhonebookEntry );

        if( phonebook.hasValue( defaultPhonebookEntry ) )
        {
            name = defaultPhonebookEntry;
            number = phonebook[defaultPhonebookEntry][_number];
            return true;
        }
    }

    return false;
}

void Phonebook::setDefaultEntry( const String& name )
{
    m_configurationStore.get( _defaultPhonebookEntry ) = name;

    m_configurationStore.save();
}

bool Phonebook::requiresAuthentication( const String& name ) const
{
    if( m_configurationStore.hasKey( _phonebook ) )
    {
        Value& phonebook( m_configurationStore.get( _phonebook ) );
        if( phonebook.hasValue( name ) )
        {
            const Value& entry( phonebook[name] );
            return entry[_authenticate];
        }
    }

    return false;
}

} // namespace Agape
