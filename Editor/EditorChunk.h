#ifndef AGAPE_EDITOR_CHUNK_H
#define AGAPE_EDITOR_CHUNK_H

#include "String.h"

namespace Agape
{

namespace Editor
{

class Chunk
{
public:
    enum Type
    {
        file,
        inserted
    };

    enum Type m_type;
    int m_offset;
    int m_len;
    String m_text;
};

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_CHUNK_H
