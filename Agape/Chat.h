#ifndef AGAPE_CHAT_H
#define AGAPE_CHAT_H

#include "Actors/NativeActors/NativeActor.h"
#include "Utils/Cartesian.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "String.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace Encryptors
{
class Factory;
} // namespace Encryptors

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

class Platform;

namespace UI
{
namespace Forms
{
class Form;
} // namespace Forms
} // namespace UI

namespace World
{
    class Metadata;
    class User;
} // namespace World

using namespace UI;

class Encryptor;
class Terminal;
class WindowManager;

class Chat : public Actors::Native
{
public:
    Chat( Coordinates& coordinates,
          Metadata& worldMetadata,
          User& worldUser,
          Encryptors::Factory& encryptorFactory,
          TupleRouter& tupleRouter,
          WindowManager& windowManager,
          const String& windowName,
          const Point& normalPosition,
          const Point& maximisedPosition,
          Platform& platform,
          Timers::Factory& timerFactory );
    ~Chat();

    void coordinatesChanged();
    void stop();

    void receiveMessage( const String& senderName,
                         int senderAttributes,
                         const String& message );
    
    void consumeCharacter( char c );

    void toggleMaximise();
    void maximise();
    void normalSize();

    void setUserActive( bool active );

    virtual bool accept( Tuple& tuple );

private:
    void addRouting();
    void removeRouting();

    void createForm();
    void deleteForm();

    void showForm();
    void hideForm();

    void sendMessage();

    int makeNameAttributes( int attributes );

    Coordinates& m_coordinates;
    Metadata& m_worldMetadata;
    User& m_worldUser;
    TupleRouter& m_tupleRouter;
    WindowManager& m_windowManager;
    String m_windowName;
    Point m_normalPosition;
    Point m_maximisedPosition;
    Platform& m_platform;

    Encryptor* m_encryptor;

    Terminal* m_terminal;

    Timer* m_notificationLastSent;

    bool m_havePreviousCoordinates;
    Coordinates m_previousCoordinates;

    Forms::Form* m_currentForm;

    bool m_lineEmpty;
    int m_cursorPos;

    bool m_isMaximised;

    bool m_userActive;
    bool m_didSendNotification;
};

} // namespace Agape

#endif // AGAPE_CHAT_H
