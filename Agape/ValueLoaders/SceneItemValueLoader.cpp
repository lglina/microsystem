#include "World/Compositor.h"
#include "SceneItemValueLoader.h"
#include "Value.h"

namespace Agape
{

namespace ValueLoaders
{

SceneItem::SceneItem( Compositor& compositor,
                      const String& snowflake,
                      const String& attributeName ) :
  m_compositor( compositor ),
  m_snowflake( snowflake ),
  m_attributeName( attributeName )
{
}

bool SceneItem::load( Value& value )
{
    return m_compositor.loadSceneItemAttribute( m_snowflake, m_attributeName, value );
}

bool SceneItem::save( const Value& value )
{
    return m_compositor.saveSceneItemAttribute( m_snowflake, m_attributeName, value );
}

} // namespace ValueLoaders

} // namespace Agape
