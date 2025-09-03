#include "Clocks/Clock.h"
#include "Loggers/Logger.h"
#include "Asset.h"
#include "SAUCE.h"
#include "String.h"

#include <string.h>

// SAUCE v00.5
// Credz to Olivier "Tasmaniac" Reubens and the ACiD crew.

namespace
{
    const char SAUCEID[] = "SAUCE";
    const char commentID[] = "COMNT";
    const char SUB = '\x1a';
}

namespace Agape
{

namespace Assets
{

bool SAUCE::extractSAUCE( Asset& asset, SAUCE& sauce )
{
    bool success( true );

    if( asset.size() >= 128 )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Attempting to read SAUCE" );
#endif
        char rawData[128];
        char* dataPtr = rawData;

        if( ( asset.readAll( rawData, asset.size() - 128, 128 ) == 128 ) &&
            ( ::memcmp( dataPtr, SAUCEID, ::strlen( SAUCEID ) ) == 0 ) )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "SAUCE found" );
#endif
            ::memcpy( sauce.m_id, dataPtr, ::strlen( SAUCEID ) ); dataPtr += ::strlen( SAUCEID );
            ::memcpy( sauce.m_version, dataPtr, 2 ); dataPtr += 2;
            ::memcpy( sauce.m_title, dataPtr, 35 ); dataPtr += 35;
            ::memcpy( sauce.m_author, dataPtr, 20 ); dataPtr += 20;
            ::memcpy( sauce.m_group, dataPtr, 20 ); dataPtr += 20;
            ::memcpy( sauce.m_date, dataPtr, 8 ); dataPtr += 8;
            ::memcpy( &sauce.m_fileSize, dataPtr, 4 ); dataPtr += 4;
            ::memcpy( &sauce.m_dataType, dataPtr, 1 ); ++dataPtr;
            ::memcpy( &sauce.m_fileType, dataPtr, 1 ); ++dataPtr;
            ::memcpy( &sauce.m_tInfo1, dataPtr, 2 ); dataPtr += 2;
            ::memcpy( &sauce.m_tInfo2, dataPtr, 2 ); dataPtr += 2;
            ::memcpy( &sauce.m_tInfo3, dataPtr, 2 ); dataPtr += 2;
            ::memcpy( &sauce.m_tInfo4, dataPtr, 2 ); dataPtr += 2;
            ::memcpy( &sauce.m_numCommentLines, dataPtr, 1 ); ++dataPtr;
            ::memcpy( &sauce.m_tFlags, dataPtr, 1 ); ++dataPtr;
            ::memcpy( sauce.m_tInfoS, dataPtr, 22 ); dataPtr += 22;
            sauce.m_tInfoS[21] = '\0'; // Should already be null-terminated, but ensure it is!

            if( sauce.m_numCommentLines > 0 )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "Attempting to read comment lines" );
#endif
                int commentBlockSize( sauce.m_numCommentLines * 64 );
                int sauceTotalSize( 128 + commentBlockSize + ::strlen( commentID ) );

                char* commentIDin = new char[ ::strlen( commentID ) ];
                if( ( asset.size() >= sauceTotalSize ) &&
                    ( asset.readAll( commentIDin,
                                     asset.size() - sauceTotalSize,
                                     ::strlen( commentID ) ) == static_cast< int >( ::strlen( commentID ) ) ) )
                {
                    if( ::memcmp( commentIDin, commentID, ::strlen( commentID ) ) == 0 )
                    {
                        // FIXME: Should we avoid dynamic memory allocation here by assuming max comment lines?
#ifdef NOTHROW
                        sauce.m_comments = new ( std::nothrow ) char[ commentBlockSize ];
#else
                        sauce.m_comments = new char[commentBlockSize];
#endif
                        success = ( asset.readAll( sauce.m_comments,
                                                   asset.size() - 128 - commentBlockSize,
                                                   commentBlockSize ) == commentBlockSize );
                    }
                    else
                    {
                        // Comment header corrupt.
                        success = false;
#ifdef LOG_LOADERS
                        LOG_DEBUG( "Comment header corrupt" );
#endif
                    }
                }
                else
                {
                    // Error reading comment header or comment line count corrupt.
                    success = false;
#ifdef LOG_LOADERS
                    LOG_DEBUG( "Error reading comment header or comment line count corrupt" );
#endif
                }
                delete[]( commentIDin );
            }
        }
        else
        {
            // Bad read or no SAUCE header.
            success = false;
#ifdef LOG_LOADERS
            LOG_DEBUG( "Bad read or no SAUCE header" );
#endif
        }

#ifdef LOG_LOADERS
        LOG_DEBUG( "Done reading SAUCE" );
#endif
    }
    else
    {
        // Asset too short for SAUCE.
        success = false;
#ifdef LOG_LOADERS
        LOG_DEBUG( "Asset too short for SAUCE" );
#endif
    }

    return success;
}

SAUCE::SAUCE() :
  m_fileSize( 0 ),
  m_dataType( 0 ),
  m_fileType( 0 ),
  m_tInfo1( 0 ),
  m_tInfo2( 0 ),
  m_tInfo3( 0 ),
  m_tInfo4( 0 ),
  m_numCommentLines( 0 ),
  m_tFlags( 0 ),
  m_comments( nullptr )
{
    ::memcpy( m_id, SAUCEID, 5 );
    ::memset( m_version, '0', 2 );
    ::memset( m_title, ' ', 35 );
    ::memset( m_author, ' ', 20 );
    ::memset( m_group, ' ', 20 );
    ::memset( m_date, ' ', 8 );
    ::memset( m_tInfoS, '\0', 22 );
}

SAUCE::SAUCE( const SAUCE& other )
{
    *this = other;
}

SAUCE::~SAUCE()
{
    delete[]( m_comments );
}

String SAUCE::title() const
{
    return String( m_title, 35 );
}

String SAUCE::author() const
{
    return String( m_author, 20 );
}

String SAUCE::group() const
{
    return String( m_group, 20 );
}

String SAUCE::date() const
{
    return String( m_date, 8 );
}

int SAUCE::fileSize() const
{
    return m_fileSize;
}

int SAUCE::dataType() const
{
    return m_dataType;
}

int SAUCE::fileType() const
{
    return m_fileType;
}

int SAUCE::tInfo1() const
{
    return m_tInfo1;
}

int SAUCE::tInfo2() const
{
    return m_tInfo2;
}

int SAUCE::tInfo3() const
{
    return m_tInfo3;
}

int SAUCE::tInfo4() const
{
    return m_tInfo4;
}

int SAUCE::tFlags() const
{
    return m_tFlags;
}

String SAUCE::tInfoS() const
{
    return String( m_tInfoS );
}

String SAUCE::comments() const
{
    if( m_numCommentLines > 0 )
    {
        return String( m_comments, m_numCommentLines * 64 );
    }
    else
    {
        return String();
    }
}

void SAUCE::SAUCE::setTitle( const String& title )
{
    ::memset( m_title, ' ', 35 );
    int lenToCopy( title.length() > 35 ? 35 : title.length() );
    ::memcpy( m_title, title.c_str(), lenToCopy );
}

void SAUCE::setAuthor( const String& author )
{
    ::memset( m_author, ' ', 20 );
    int lenToCopy( author.length() > 20 ? 20 : author.length() );
    ::memcpy( m_author, author.c_str(), lenToCopy );
}

void SAUCE::setGroup( const String& group )
{
    ::memset( m_group, ' ', 20 );
    int lenToCopy( group.length() > 20 ? 20 : group.length() );
    ::memcpy( m_group, group.c_str(), lenToCopy );
}

void SAUCE::setFileSize( int fileSize )
{
    m_fileSize = fileSize;
}

void SAUCE::setDataType( int dataType )
{
    m_dataType = dataType;
}

void SAUCE::setFileType( int fileType )
{
    m_fileType = fileType;
}

void SAUCE::setTInfo1( int tInfo1 )
{
    m_tInfo1 = tInfo1;
}

void SAUCE::setTInfo2( int tInfo2 )
{
    m_tInfo2 = tInfo2;
}

void SAUCE::setTInfo3( int tInfo3 )
{
    m_tInfo3 = tInfo3;
}

void SAUCE::setTInfo4( int tInfo4 )
{
    m_tInfo4 = tInfo4;
}

void SAUCE::setTFlags( int tFlags )
{
    m_tFlags = tFlags;
}

void SAUCE::setTInfoS( const String& tInfoS )
{
    ::strncpy( m_tInfoS, tInfoS.c_str(), 21 );
    m_tInfoS[21] = '\0'; // Ensure null-terminated.
}

void SAUCE::setComments( const String& comments )
{
    delete[]( m_comments );
    
    int inputCommentLines( comments.length() / 64 );
    if( ( comments.length() % 64 ) != 0 ) ++inputCommentLines;
    m_numCommentLines = ( inputCommentLines > 255 ) ? 255 : inputCommentLines; // Only 255 supported.

    int commentBlockSize( m_numCommentLines * 64 );
    m_comments = new char[commentBlockSize];

    for( int lineNum = 0; lineNum < m_numCommentLines; ++lineNum )
    {
        int startIndex( lineNum * 64 );
        int stringRemain( comments.length() - startIndex );
        int paddingBytes( 64 - stringRemain );
        ::memcpy( m_comments + startIndex, comments.c_str() + startIndex, stringRemain );
        if( paddingBytes > 0 ) ::memset( m_comments + startIndex + stringRemain, ' ', paddingBytes );
    }
}

void SAUCE::setDateFromClock( Clock& clock )
{
    String dateTime( clock.dateTime() );
    if( dateTime.length() == 14 )
    {
        ::memcpy( m_date, dateTime.c_str(), 8 ); // Copy only date part.
    }
}

bool SAUCE::writeSAUCE( Asset& asset, int offset )
{
    bool success( true );

    success = ( asset.write( &SUB, offset, 1 ) == 1 );
    ++offset;

    if( success && ( m_numCommentLines > 0 ) )
    {
        success = ( asset.write( commentID, offset, ::strlen( commentID ) ) == ::strlen( commentID ) );
        if( success )
        {
            offset += ::strlen( commentID );

            for( int lineNum = 0; success && ( lineNum < m_numCommentLines ); ++lineNum )
            {
                success = ( asset.write( m_comments + ( lineNum * 64 ), offset, 64 ) == 64 );
                offset += 64;
            }
        }
    }

    if( success )
    {
        success = ( asset.write( SAUCEID, offset, ::strlen( SAUCEID ) ) == ::strlen( SAUCEID ) );
        offset += ::strlen( SAUCEID );
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_version, offset, 2 ) == 2 );
        offset += 2;
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_title, offset, 35 ) == 35 );
        offset += 35;
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_author, offset, 20 ) == 20 );
        offset += 20;
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_group, offset, 20 ) == 20 );
        offset += 20;
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_date, offset, 8 ) == 8 );
        offset += 8;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_fileSize, offset, 4 ) == 4 );
        offset += 4;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_dataType, offset, 1 ) == 1 );
        ++offset;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_fileType, offset, 1 ) == 1 );
        ++offset;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_tInfo1, offset, 2 ) == 2 );
        offset += 2;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_tInfo2, offset, 2 ) == 2 );
        offset += 2;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_tInfo3, offset, 2 ) == 2 );
        offset += 2;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_tInfo4, offset, 2 ) == 2 );
        offset += 2;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_numCommentLines, offset, 1 ) == 1 );
        ++offset;
    }

    if( success )
    {
        success = ( asset.write( (const char*)&m_tFlags, offset, 1 ) == 1 );
        ++offset;
    }

    if( success )
    {
        success = ( asset.write( (const char*)m_tInfoS, offset, 22 ) == 22 );
        offset += 22;
    }

    return success;
}

SAUCE& SAUCE::operator=( const SAUCE& other )
{
    ::memcpy( m_id, other.m_id, 5 );
    ::memcpy( m_version, other.m_version, 2 );
    ::memcpy( m_title, other.m_title, 35 );
    ::memcpy( m_author, other.m_author, 20 );
    ::memcpy( m_group, other.m_group, 20 );
    ::memcpy( m_date, other.m_date, 8 );
    m_fileSize = other.m_fileSize;
    m_dataType = other.m_dataType;
    m_fileType = other.m_fileType;
    m_tInfo1 = other.m_tInfo1;
    m_tInfo2 = other.m_tInfo2;
    m_tInfo3 = other.m_tInfo3;
    m_tInfo4 = other.m_tInfo4;
    m_numCommentLines = other.m_numCommentLines;
    m_tFlags = other.m_tFlags;
    ::memcpy( m_tInfoS, other.m_tInfoS, 22 );

    delete[]( m_comments );

    if( m_numCommentLines > 0 )
    {
        int commentBlockSize( m_numCommentLines * 64 );
        m_comments = new char[commentBlockSize];
        ::memcpy( m_comments, other.m_comments, commentBlockSize );
    }

    return *this;
}

} // namespace Assets

} // namespace Agape
