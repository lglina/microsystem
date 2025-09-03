#ifndef AGAPE_SCENEITEM_VALUE_LOADER_H
#define AGAPE_SCENEITEM_VALUE_LOADER_H

#include "String.h"
#include "ValueLoader.h"

namespace Agape
{

namespace World
{
class Compositor;
} // namespace World

using namespace World;

class Value;

namespace ValueLoaders
{

/// Uses Compositor to load and save attributes from/to scene items
/// using Compositor's current SceneLoader.
class SceneItem : public ValueLoader
{
public:
    SceneItem( Compositor& compositor,
               const String& snowflake,
               const String& attributeName );

    virtual bool load( Value& value );
    virtual bool save( const Value& value );

private:
    Compositor& m_compositor;
    String m_snowflake;
    String m_attributeName;
};

} // namespace ValueLoaders

} // namespace Agape

#endif // AGAPE_SCENEITEM_VALUE_LOADER_H
