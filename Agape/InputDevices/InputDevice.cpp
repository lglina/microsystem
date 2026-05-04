#include "InputDevice.h"

namespace Agape
{

InputDevice::InputDevice() :
  m_peekEnabled( true )
{
}

bool InputDevice::peekEnabled()
{
    return m_peekEnabled;
}

void InputDevice::setPeekEnabled( bool enabled )
{
    m_peekEnabled = enabled;
}

} // namespace Agape
