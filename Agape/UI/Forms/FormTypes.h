#ifndef AGAPE_UI_FORMS_TYPES_H
#define AGAPE_UI_FORMS_TYPES_H

#include "String.h"

namespace Agape
{

namespace UI
{

namespace Forms
{

struct Title
{
    Title();

    Title( const String& title, int row, int col, int width, bool centre, int attributes );

    String m_title;
    int m_row;
    int m_col;
    int m_width;
    bool m_centre;
    int m_attributes;
};

enum FieldType
{
    fText,
    fSelect,
    fButton,
    fUnknown
};

struct XProps
{
    virtual ~XProps();
};

struct TextXProps : public XProps
{
    TextXProps( int maxLength );

    int m_maxLength;
};

struct SelectXProps : public XProps
{
    SelectXProps( int selectionAttributes, char delim, int selectionNotFocusedAttributes );

    int m_numValues;
    int m_selection;
    int m_topValue;
    int m_selectionAttributes;
    int m_selectionNotFocusedAttributes;
    char m_delim;
};

struct ButtonXProps : public XProps
{
    ButtonXProps( int selectionAttributes );

    int m_selectionAttributes;
};

struct Field
{
    Field();

    Field( const Field& other );

    /// @brief For text.
    Field( const String& name,
           int width,
           int height,
           int row,
           int col,
           int fieldAttributes,
           int labelAttributes = 0 );
    
    /// @brief For text.
    Field( const String& name,
           int width,
           int height,
           int row,
           int col,
           int fieldAttributes,
           int labelAttributes,
           int labelRow,
           int labelCol,
           int maxLength = -1 );

    /// @brief For select.
    Field( const String& name,
            int width,
            int height,
            int row,
            int col,
            int fieldAttributes,
            const String& values,
            int selectionAttributes,
            int labelAttributes = 0,
            char delim = ';',
            int selectionNotFocusedAttributes = 0x8F );

    /// @brief For buttons.
    Field( const String& name,
            int row,
            int col,
            int labelAttributes,
            int selectionAttributes );

    ~Field();

    enum FieldType m_type;
    String m_name;
    String m_contents;
    String m_permittedSymbols;
    int m_width;
    int m_height;
    bool m_enabled;
    int m_row;
    int m_col;
    int m_labelRow;
    int m_labelCol;
    int m_labelAttributes;
    int m_fieldAttributes;
    XProps* m_xprops;
};

} // namespace Forms

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_FORMS_TYPES_H
