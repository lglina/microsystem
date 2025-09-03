#ifndef AGAPE_WORLDBOOK_H
#define AGAPE_WORLDBOOK_H

#include "Collections.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

namespace World
{
class Metadata;
class User;
} // namespace World

class ConfigurationStore;

class Worldbook
{
public:
    Worldbook( ConfigurationStore& configurationStore );

    void add( const Metadata& metadata, bool setDefault = true );
    void remove( const String& worldID );

    Vector< String > getNames() const;
    Vector< String > getIDs() const;
    bool getMetadata( const String& worldID, Metadata& metadata ) const;
    Vector< Metadata > getAllMetadata() const;

    bool getWorldIDByName( const String& name, String& worldID ) const;
    bool getWorldNameByID( const String& worldID, String& name ) const;

    bool hasDefaultWorldID() const;
    String getDefaultWorldID() const;
    void setDefaultWorldID( const String& worldID );

    bool getUserForWorld( const String& worldID, User& user ) const;
    void setUserForWorld( const String& worldID, const User& user );
    void removeUserForWorld( const String& worldID );

private:
    ConfigurationStore& m_configurationStore;
};

} // namespace Agape

#endif // AGAPE_WORLDBOOK_H
