#include "FormTypes.h"
#include "String.h"

namespace Agape
{

namespace UI
{

namespace Forms
{

Title::Title() :
  m_row( 0 ),
  m_col( 0 ),
  m_width( 0 ),
  m_centre( false ),
  m_attributes( 0 )
{
}

Title::Title( const String& title,
              int row,
              int col,
              int width,
              bool centre,
              int attributes ) :
  m_title( title ),
  m_row( row ),
  m_col( col ),
  m_width( width ),
  m_centre( centre ),
  m_attributes( attributes )
{
}

XProps::~XProps()
{
}

TextXProps::TextXProps( int maxLength ) :
  m_maxLength( maxLength )
{
}

SelectXProps::SelectXProps( int selectionAttributes, char delim, int selectionNotFocusedAttributes ) :
  m_numValues( 0 ),
  m_selection( 0 ),
  m_topValue( -1 ),
  m_selectionAttributes( selectionAttributes ),
  m_selectionNotFocusedAttributes( selectionNotFocusedAttributes ),
  m_delim( delim )
{
}

ButtonXProps::ButtonXProps( int selectionAttributes ) :
  m_selectionAttributes( selectionAttributes )
{
}

Field::Field() :
  m_type( fUnknown ),
  m_width( -1 ),
  m_height( -1 ),
  m_enabled( true ),
  m_row( -1 ),
  m_col( -1 ),
  m_labelRow( -1 ),
  m_labelCol( -1 ),
  m_labelAttributes( 0 ),
  m_fieldAttributes( 0 ),
  m_xprops( nullptr )
{
}

Field::Field( const Field& other ) :
  m_type( other.m_type ),
  m_name( other.m_name ),
  m_contents( other.m_contents ),
  m_permittedSymbols( other.m_permittedSymbols ),
  m_width( other.m_width ),
  m_height( other.m_height ),
  m_enabled( other.m_enabled ),
  m_row( other.m_row ),
  m_col( other.m_col ),
  m_labelRow( other.m_labelRow ),
  m_labelCol( other.m_labelCol ),
  m_labelAttributes( other.m_labelAttributes ),
  m_fieldAttributes( other.m_fieldAttributes ),
  m_xprops( nullptr )
{
    if( m_type == fText )
    {
        m_xprops = new TextXProps( *( static_cast< TextXProps* >( other.m_xprops ) ) );
    }
    if( m_type == fSelect )
    {
        m_xprops = new SelectXProps( *( static_cast< SelectXProps* >( other.m_xprops ) ) );
    }
    else if( m_type == fButton )
    {
        m_xprops = new ButtonXProps( *( static_cast< ButtonXProps* >( other.m_xprops ) ) );
    }
}

/// @brief For text fields.
Field::Field( const String& name,
              int width,
              int height,
              int row,
              int col,
              int fieldAttributes,
              int labelAttributes ) :
  m_type( fText ),
  m_name( name ),
  m_width( width ),
  m_height( height ),
  m_enabled( true ),
  m_row( labelAttributes == 0 ? row : row + 1 ),
  m_col( col ),
  m_labelRow( labelAttributes == 0 ? -1 : row ),
  m_labelCol( labelAttributes == 0 ? -1 : col ),
  m_labelAttributes( labelAttributes ),
  m_fieldAttributes( fieldAttributes ),
  m_xprops( new TextXProps( width ) )
{
}

/// @brief For text fields.
Field::Field( const String& name,
              int width,
              int height,
              int row,
              int col,
              int fieldAttributes,
              int labelAttributes,
              int labelRow,
              int labelCol,
              int maxLength ) :
  m_type( fText ),
  m_name( name ),
  m_width( width ),
  m_height( height ),
  m_enabled( true ),
  m_row( row ),
  m_col( col ),
  m_labelRow( labelRow ),
  m_labelCol( labelCol ),
  m_labelAttributes( labelAttributes ),
  m_fieldAttributes( fieldAttributes ),
  m_xprops( new TextXProps( maxLength == -1 ? width * height : maxLength ) )
{
}

/// @brief For select lists.
Field::Field( const String& name,
              int width,
              int height,
              int row,
              int col,
              int fieldAttributes,
              const String& values,
              int selectionAttributes,
              int labelAttributes,
              char delim,
              int selectionNotFocusedAttributes ) :
  m_type( fSelect ),
  m_name( name ),
  m_contents( values ),
  m_width( width ),
  m_height( height ),
  m_enabled( true ),
  m_row( labelAttributes == 0 ? row : row + 1 ),
  m_col( col ),
  m_labelRow( labelAttributes == 0 ? -1 : row ),
  m_labelCol( labelAttributes == 0 ? -1 : col ),
  m_labelAttributes( labelAttributes ),
  m_fieldAttributes( fieldAttributes ),
  m_xprops( new SelectXProps( selectionAttributes, delim, selectionNotFocusedAttributes ) )
{
}

/// @brief For buttons.
Field::Field( const String& name,
              int row,
              int col,
              int labelAttributes,
              int selectionAttributes ) :
  m_type( fButton ),
  m_name( name ),
  m_width( -1 ),
  m_height( -1 ),
  m_enabled( true ),
  m_row( -1 ),
  m_col( -1 ),
  m_labelRow( row ),
  m_labelCol( col ),
  m_labelAttributes( labelAttributes ),
  m_fieldAttributes( 0 ),
  m_xprops( new ButtonXProps( selectionAttributes ) )
{
}

Field::~Field()
{
    delete( m_xprops );
}

} // namespace Forms

} // namespace UI

} // namespace Agape
