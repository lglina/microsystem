#ifndef AGAPE_LINDA2_TUPLE_FILTER_H
#define AGAPE_LINDA2_TUPLE_FILTER_H

namespace Agape
{

namespace Linda2
{

class Tuple;

class TupleFilter
{
public:
    virtual bool permitIn( const Tuple& tuple ) = 0;
    virtual bool permitInDefault( const Tuple& tuple ) = 0;
    virtual bool permitForward( const Tuple& tuple ) = 0;
    virtual bool permitForwardDefault( const Tuple& tuple ) = 0;
    virtual bool permitOut( const Tuple& tuple ) = 0;
    virtual bool permitOutDefault( const Tuple& tuple ) = 0;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_FILTER_H
