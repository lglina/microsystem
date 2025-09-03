#ifndef AGAPE_CARLO_EXPRESSION_H
#define AGAPE_CARLO_EXPRESSION_H

#include "SyntaxTreeNode.h"

namespace Agape
{

namespace Carlo
{

class Expression : public SyntaxTreeNode
{
public:
    virtual ~Expression() {}
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXPRESSION_H
