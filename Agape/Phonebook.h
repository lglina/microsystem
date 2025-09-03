#ifndef AGAPE_PHONEBOOK_H
#define AGAPE_PHONEBOOK_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

class ConfigurationStore;

class Phonebook
{
public:
    Phonebook( ConfigurationStore& configurationStore );

    void add( const String& name, const String& number, bool setDefault = true );
    void remove( const String& name );

    Map< String, String > getEntries() const;

    bool hasDefaultEntry() const;
    bool getDefaultEntry( String& name, String& number ) const;
    void setDefaultEntry( const String& name );

    bool requiresAuthentication( const String& name ) const;

private:
    ConfigurationStore& m_configurationStore;
};

} // namespace Agape

#endif // AGAPE_PHONEBOOK_H
