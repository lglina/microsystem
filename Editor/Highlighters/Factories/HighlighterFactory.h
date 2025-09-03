#ifndef AGAPE_EDITOR_HIGHLIGHTERS_FACTORY_H
#define AGAPE_EDITOR_HIGHLIGHTERS_FACTORY_H

#include "Highlighters/Highlighter.h"

namespace Agape
{

namespace Editor
{

namespace Highlighters
{

class Factory
{
public:
    virtual ~Factory() {};

    virtual Highlighter* makeHighlighter() = 0;
};

} // namespace Highlighters

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_HIGHLIGHTERS_FACTORY_H
