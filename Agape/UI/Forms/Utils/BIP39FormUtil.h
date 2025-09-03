#ifndef AGAPE_UI_FORMS_UTILS_BIP39_H
#define AGAPE_UI_FORMS_UTILS_BIP39_H

#include "Collections.h"

namespace Agape
{

class Terminal;

namespace UI
{

namespace Forms
{

class Field;
class Form;

namespace Utils
{

class BIP39
{
public:
    static void showKey( const char* key,
                         Terminal* terminal,
                         int firstRow,
                         int firstCol,
                         int attributes );

    static void createKeyForm( Vector< Forms::Field >& formFields,
                               int firstRow,
                               int firstCol,
                               int attributes,
                               char* key = nullptr );

    static bool getKey( Form* form, char*& key );

    static void tryAutoComplete( Form* form );
};

} // namespace Utils

} // namespace Forms

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_FORMS_UTILS_BIP39_H
