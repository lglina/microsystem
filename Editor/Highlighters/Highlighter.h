#ifndef AGAPE_EDITOR_HIGHLIGHTER_H
#define AGAPE_EDITOR_HIGHLIGHTER_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Editor
{

class Highlighter
{
public:
    virtual ~Highlighter() {}

    struct Token
    {
        int m_line;
        int m_column;
        int m_length;
        int m_attributes;
    };

    virtual void line( const String& line, int logicalLine ) = 0;
    virtual void highlight( const String& instanceName,
                            bool modified,
                            Vector< Token >& highlights,
                            Vector< String >& parseErrors,
                            Vector< String >& runtimeErrors ) = 0;
};

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_HIGHLIGHTER_H
