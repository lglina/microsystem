#ifndef AGAPE_CARLO_STATEMENT_H
#define AGAPE_CARLO_STATEMENT_H

#include "SyntaxTreeNode.h"

namespace Agape
{

namespace Carlo
{

class Statement : public SyntaxTreeNode
{
public:
    virtual ~Statement() {}
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENT_H
