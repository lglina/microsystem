#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "UI/Forms/Form.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "ModemConfigStrategy.h"
#include "String.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int selectWidth( 30 );
    const int selectHeight( 5 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    const int labelAttributes( 0x07 );
    const int textWidth( 50 );
    const int textHeight( 1 );

    const String backgroundAssetName( "welcome" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

ModemConfig::ModemConfig( InputDevice& inputDevice,
                          WindowManager& windowManager,
                          const String& windowName,
                          Line& line ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_line( line ),
  m_completed( false ),
  m_currentForm( nullptr )
{
}

ModemConfig::~ModemConfig()
{
    closeForm();
}

void ModemConfig::enter( const Value& parameters )
{
    m_completed = false;
    
    drawConfigureModem();
}

void ModemConfig::returnTo( const Value& parameters )
{
}

bool ModemConfig::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool ModemConfig::returning( String& nextStrategy, Value& parameters )
{
    closeForm();
    nextStrategy = m_nextStrategy;
    return m_completed;
}

void ModemConfig::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == '\n' )
        {
            configureModem();
            m_completed = true;
        }
        else if( c == '\x1b' )
        {
            m_completed = true;
        }
        else
        {
            m_currentForm->consumeChar( c );
        }
        break;
    }
}

void ModemConfig::drawConfigureModem()
{
    if( m_currentForm != nullptr )
    {
        Vector< Forms::Field > configOptionsFields;
        
        if( m_modemConfigOptions.empty() )
        {
            m_modemConfigOptions = m_line.getConfigOptions();
        }

        int currentRow( contentFirstRow );
        Vector< Line::ConfigOption >::const_iterator it( m_modemConfigOptions.begin() );
        for( ; it != m_modemConfigOptions.end(); ++it )
        {
            if( it->m_type == Line::select )
            {
                Forms::Field thisField( it->m_name,
                                        selectWidth,
                                        selectHeight,
                                        currentRow,
                                        contentFirstCol,
                                        fieldAttributes,
                                        it->m_alternatives,
                                        selectionAttributes,
                                        labelAttributes );
                configOptionsFields.push_back( thisField );

                currentRow += selectHeight + 2;
            }
            else if( it->m_type == Line::text ||
                    it->m_type == Line::encodedText )
            {
                Forms::Field thisField( it->m_name,
                                        selectWidth,
                                        textHeight,
                                        currentRow,
                                        contentFirstCol,
                                        fieldAttributes,
                                        labelAttributes );
                configOptionsFields.push_back( thisField );

                currentRow += textHeight + 2;
            }
        }

        closeForm();

        struct Forms::Title title;
        m_currentForm = new Forms::Form( m_windowManager,
                                m_windowName,
                                backgroundAssetName,
                                configOptionsFields,
                                title );
        m_currentForm->openUI();

        if( m_currentForm->uiIsOpen() )
        {
            m_currentForm->draw();
        }
    }
}

void ModemConfig::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void ModemConfig::configureModem()
{
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
