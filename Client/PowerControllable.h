#ifndef AGAPE_POWER_CONTROLLABLE_H
#define AGAPE_POWER_CONTROLLABLE_H

namespace Agape
{

class PowerControllable
{
public:
    enum PowerState
    {
        off,
        on
    };

    virtual void setPowerState( enum PowerState powerState ) = 0;
};

} // namespace Agape

#endif // AGAPE_POWER_CONTROLLABLE_H
