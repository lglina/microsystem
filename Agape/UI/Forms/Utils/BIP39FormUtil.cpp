#include "Encryptors/Utils/BIP39/Mnemonic.h"
#include "InputDevices/InputDevice.h"
#include "UI/Forms/Form.h"
#include "BIP39FormUtil.h"
#include "Collections.h"
#include "String.h"
#include "Terminal.h"

using namespace Agape::Encryptors::Utils::BIP39;
using namespace Agape::InputDevices;

namespace
{
    const int keyFieldWidth( 8 );
    const int keyFieldHeight( 1 );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Forms
{

namespace Utils
{

void BIP39::showKey( const char* key,
                     Terminal* terminal,
                     int firstRow,
                     int firstCol,
                     int attributes )
{
    Vector< String > words;
    Mnemonic::encode( key, words );

    for( int i = 0; i < 12; ++i )
    {
        terminal->consumeNext( firstRow + 4 + ( ( i / 6 ) * 2 ),
                                 firstCol + ( ( keyFieldWidth + 1 ) * ( i % 6 ) ),
                                 attributes );
        terminal->consumeString( words[i], false, Terminal::preserveBackground );
    }
}

void BIP39::createKeyForm( Vector< Forms::Field >& formFields,
                           int firstRow,
                           int firstCol,
                           int attributes,
                           char* key )
{
    bool fillFields( false );
    Vector< String > words;
    if( key )
    {
        if( Mnemonic::encode( key, words ) )
        {
            fillFields = true;
        }
    }

    char id;
    for( int i = 0; i < 2; ++i )
    {
        for( int j = 0; j < 6; ++j )
        {
            id = 'a' + ( 6 * i ) + j;
            Forms::Field field( String( &id, 1 ),
                                        keyFieldWidth,
                                        keyFieldHeight,
                                        firstRow + ( i * 2 ),
                                        firstCol + ( j * ( keyFieldWidth + 1 ) ),
                                        attributes );
            
            if( fillFields )
            {
                field.m_contents = words[( 6 * i ) + j];
            }
            
            formFields.push_back( field );
        }
    }
}

bool BIP39::getKey( Form* form, char*& key )
{
    Vector< String > words;
    char id;
    for( int i = 0; i < 12; ++i )
    {
        id = 'a' + i;
        words.push_back( form->getFieldContents( String( &id, 1 ) ) );
    }

    return( Mnemonic::decode( words, key ) );
}

void BIP39::tryAutoComplete( Form* form )
{
    const Forms::Field& currentField( form->currentField() );
    if( ( currentField.m_name[0] >= 'a' && currentField.m_name[0] <= 'l' ) &&
        ( currentField.m_contents.length() >= 3 ) )
    {
        String completion;
        if( Mnemonic::completeWord( currentField.m_contents, completion ) )
        {
            if( completion != currentField.m_contents )
            {
                for( int i = currentField.m_contents.length(); i < completion.length(); ++i )
                {
                    form->consumeChar( completion[i] );
                }
            }

            if( ( completion.length() == currentField.m_contents.length() ) && ( currentField.m_name[0] != 'l' ) )
            {
                form->consumeChar( Key::tab ); // Next field, if not at end.
            }
        }
    }
}

} // namespace Utils

} // namespace Forms

} // namespace UI

} // namespace Agape
