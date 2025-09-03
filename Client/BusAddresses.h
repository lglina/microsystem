#ifndef AGAPE_BUS_ADDRESSES_H
#define AGAPE_BUS_ADDRESSES_H

namespace Agape
{

namespace BusAddresses
{

enum BusAddresses
{
    None            = 0x0000,

    // 0x0000 - 0x000F: Chip selects
    CSEstelle       = 0x0001,
    CSFlash         = 0x0002,

    CSX1            = 0x0008,
    CSX2            = 0x0009,

    // 0x0010 - 0x001F: Graphics
    GraphCommand    = 0x0010,
    GraphData       = 0x0011,
    GraphNotReset   = 0x001E,
    GraphReset      = 0x001F,

    // 0x0020 - 0x002F: Sound
    MIDIOut         = 0x0020

    // 0x0030 - 0x007F: Reserved for Agape-branded peripherals

    // 0x0080 - 0x00FE: User/third-party peripherals

    // 0xFF: Reserved
};

} // namespace BusAddresses

} // namespace Agape

#endif // AGAPE_BUS_ADDRESSES_H
