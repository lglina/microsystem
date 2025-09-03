#ifndef AGAPE_CHOOSER_H
#define AGAPE_CHOOSER_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

class ClientBuilder;

class Chooser
{
public:
    Chooser();
    ~Chooser();

    void addChoice( ClientBuilder* clientBuilder, const String& name );

    bool setCurrentChoice( const String& name );
    const String& currentChoiceName();
    ClientBuilder* currentChoice();
    void nextChoice();

    bool changed();
    void resetChanged();

private:
    Map< String, ClientBuilder* > m_builders;
    Map< String, ClientBuilder* >::const_iterator m_currentChoice;
    bool m_changed;
};

} // namespace Agape

#endif // AGAPE_CHOOSER_H
