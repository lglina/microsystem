#ifndef AGAPE_BUS_CONTROLLER_H
#define AGAPE_BUS_CONTROLLER_H

namespace Agape
{

class BusController
{
public:
    BusController();

    virtual int read( int address, char* data, int len ); // Note: Caller responsible for dummy reads. See PMP docs.
    virtual int write( int address, const char* data, int len );
    virtual bool error();

    void write( int address, int data ); // Convenience function for writing literals in LCD drivers.

    void reset();

private:
    void setAddress( int address );

    int m_address;
};

} // namespace Agape

#endif // AGAPE_BUS_CONTROLLER_H
