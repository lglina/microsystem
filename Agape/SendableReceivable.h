#ifndef AGAPE_SENDABLE_RECEIVABLE_H
#define AGAPE_SENDABLE_RECEIVABLE_H

namespace Agape
{

class SendableReceivable
{
public:
    virtual ~SendableReceivable() {}

    virtual int send( const char* data, int len ) = 0;
    virtual int recv( char* data, int len ) = 0;
};

} // namespace Agape

#endif // AGAPE_SENDABLE_RECEIVABLE_H
