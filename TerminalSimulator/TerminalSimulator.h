#ifndef AGAPE_TERMINAL_SIMULATOR_H
#define AGAPE_TERMINAL_SIMULATOR_H

#include <QObject>
#include <memory>

namespace Agape
{

class Chooser;
class Client;
class ClientBuilder;
class TerminalSimulator;

class TerminalSimulator : public QObject
{
    Q_OBJECT

public:
    TerminalSimulator( Chooser& chooser );
    ~TerminalSimulator();

private:
    Chooser& m_chooser;
    ClientBuilder* m_currentBuilder;
    Client* m_client;

public slots:
    void run();
};

} // namespace Agape

int main( int argc, char** argv );

#endif // AGAPE_TERMINAL_SIMULATOR_H
