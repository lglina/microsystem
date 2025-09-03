#ifndef AGAPE_UI_STRATEGIES_TELEGRAM_H
#define AGAPE_UI_STRATEGIES_TELEGRAM_H

#include "UI/Strategy.h"
#include "World/Telegram.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Editor
{
class Editor;
class Factory;
} // namespace Editor

namespace Encryptors
{
class Factory;
} // namespace Encryptors

namespace TelegramLoaders
{
class Factory;
} // namespace TelegramLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Metadata;
class User;
} // namespace World

using namespace World;

class Clock;
class Encryptor;
class InputDevice;
class Snowflake;
class TelegramLoader;
class Terminal;
class Timer;
class WindowManager;
class Worldbook;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Dialogue;

namespace Strategies
{

class Telegram : public Strategy
{
public:
    Telegram( InputDevice& inputDevice,
              TelegramLoaders::Factory& telegramLoaderFactory,
              const User& worldUser,
              const Metadata& worldMetadata,
              const Worldbook& worldbook,
              AssetLoaders::Factory& assetLoaderFactory,
              Agape::Editor::Factory& editorFactory,
              Encryptors::Factory& encryptorFactory,
              WindowManager& windowManager,
              const String& windowName,
              Dialogue& dialogue,
              Timers::Factory& timerFactory,
              Clock& clock,
              Snowflake& snowflake );
    ~Telegram();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        list,
        listSent,
        listError,
        read,
        readError,
        selectRecipient,
        enterSubject,
        edit,
        discard,
        saveError,
        sendSuccess,
        sendError,
        deleteSuccess,
        deleteError
    };

    void drawBackground();
    void drawLoadPending();
    bool drawList( bool sent );
    void drawListError();
    bool drawRead();
    void drawReadError();
    void drawSelectRecipient();
    void drawEnterSubject();
    bool drawEdit();
    void drawDiscard();
    void drawSaveError();
    void drawSendPending();
    void drawSendSuccess();
    void drawSendError();
    void drawDeletePending();
    void drawDeleteSuccess();
    void drawDeleteError();
    void hideDialogue();

    bool closeEditor( bool save );

    void closeForm();

    void getRecipientSnowflake();
    void getSubject();
    bool prepareReply();
    bool sendTelegram();
    bool deleteTelegram();

    InputDevice& m_inputDevice;
    TelegramLoaders::Factory& m_telegramLoaderFactory;
    const User& m_worldUser;
    const Metadata& m_worldMetadata;
    const Worldbook& m_worldBook;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Agape::Editor::Factory& m_editorFactory;
    WindowManager& m_windowManager;
    String m_windowName;
    Dialogue& m_dialogue;
    Clock& m_clock;
    Snowflake& m_snowflake;

    Encryptor* m_encryptor;

    enum State m_state;
    enum State m_prevState;
    bool m_completed;
    Timer* m_timer;

    Terminal* m_terminal;
    Forms::Form* m_currentForm;
    Agape::Editor::Editor* m_editor;

    TelegramLoader* m_telegramLoader;
    Vector< World::Telegram > m_telegrams;

    World::Telegram m_newTelegram;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_TELEGRAM_H
