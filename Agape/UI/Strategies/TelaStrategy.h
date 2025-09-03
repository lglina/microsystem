#ifndef AGAPE_STRATEGIES_TELA_H
#define AGAPE_STRATEGIES_TELA_H

#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

namespace World
{
class Utilities;
} // namespace World

class ConfigurationStore;
class InputDevice;
class KeyUtilities;
class Phonebook;
class Worldbook;
class Value;

namespace UI
{

class Dialogue;

namespace Strategies
{

class Tela : public Strategy
{
public:
    Tela( InputDevice& inputDevice,
          WorldLoaders::Factory& worldLoaderFactory,
          ConfigurationStore& configurationStore,
          World::Utilities& worldUtilities,
          KeyUtilities& keyUtilities,
          Agape::Phonebook& phonebook,
          Agape::Worldbook& worldbook,
          Dialogue& dialogue,
          const String& defaultPhoneName,
          const String& defaultPhoneNumber );

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        connect,
        chooseAvatar,
        join,
        loadError
    };

    void drawLoadPending();
    void drawLoadError();
    void hideDialogue();

    void initPhonebook();
    bool setKeys();
    bool _load();

    void getInviteeKey();
    void eraseInviteeKey();

    InputDevice& m_inputDevice;
    WorldLoaders::Factory& m_worldLoaderFactory;
    ConfigurationStore& m_configurationStore;
    World::Utilities& m_worldUtilities;
    KeyUtilities& m_keyUtilities;
    Agape::Phonebook& m_phonebook;
    Worldbook& m_worldbook;
    Dialogue& m_dialogue;
    String m_defaultPhoneName;
    String m_defaultPhoneNumber;

    enum State m_state;
    bool m_completed;
    bool m_calling;
    Value m_callingParameters;
    String m_nextStrategy;

    String m_joinKey;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_TELA_H
