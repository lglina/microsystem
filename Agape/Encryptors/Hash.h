#ifndef AGAPE_HASH_H
#define AGAPE_HASH_H

namespace Agape
{

class Hash
{
public:
    virtual ~Hash() {}
    
    virtual void update( const char* data, int len ) = 0;

    virtual int digestSize() = 0;
    virtual void finalise( char* digest ) = 0;

    virtual void reset() = 0;
};

} // namespace Agape

#endif // AGAPE_HASH_H
