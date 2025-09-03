#include "Utils/LiteStream.h"
#include "Loggers/Logger.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "Memories/Memory.h"
#include "String.h"

#include <assert.h>

#include <cstring>

namespace Agape
{

KiamaFS::File::File( const String& filename, KiamaFS& fs ) :
  m_filename( filename ),
  m_fs( fs ),
  m_openMode( notOpen ),
  m_version( -1 ),
  m_newVersion( 0 ),
  m_size( 0 ),
  m_newSize( 0 ),
  m_currentSector( -1 ),
  m_currentPageSeq( 0 ),
  m_currentPageInSector( 0 ),
  m_currentPageDataSize( 0 ),
  m_currentPageDataOffset( 0 ),
  m_writing( false )
{
}

bool KiamaFS::File::open( OpenMode openMode )
{
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Opening " << m_filename << " with mode " << openMode;
    LOG_DEBUG( stream.str() );
    }
#endif
    if( m_openMode == notOpen )
    {
        m_openMode = openMode;
    }
    else
    {
        return false;
    }

    // Work out which sectors contain the latest version of this file.
    // We do this even for write mode, so commit() can mark the existing
    // version (if any) sectors for deletion.
    getSectors();

    if( m_openMode == readMode )
    {
        // Find zero'th page.
        return findPage( 0 );
    }
    else if( m_openMode == writeMode )
    {
        if( m_version != -1 )
        {
            m_newVersion = m_version + 1;
            if( m_newVersion == 0x80 ) m_newVersion = 0x00;
        }

        if( findFreePage() )
        {
            allocatePage();
            m_newSectors.push_back( m_currentSector );
            return true;
        }
        else
        {
#ifdef KIAMA_DEBUG
            LOG_DEBUG( "Unable to find free page" );
#endif
            m_fs.m_error = true;
        }
    }

    return false;
}

int KiamaFS::File::size()
{
    if( m_openMode == readMode )
    {
        return m_size;
    }
    else
    {
        return 0;
    }
}

bool KiamaFS::File::seek( int offset )
{
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Seek to position " << offset;
    LOG_DEBUG( stream.str() );
    }
#endif
    if( m_openMode != readMode )
    {
        // We don't support random-access writes.
        // FIXME: Throw an error.
        return false;
    }

    // FIXME: Abort if offset >= file size.

    int pageSeq( offset / 255 );
    int pageDataOffset( offset % 255 );

    if( findPage( pageSeq ) && ( pageDataOffset < m_currentPageDataSize ) )
    {
        m_currentPageDataOffset = pageDataOffset;
    }
    else
    {
        return false;
    }
    
    return true;
}

int KiamaFS::File::read( char* data, int len )
{
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Read length " << len;
    LOG_DEBUG( stream.str() );
    }
#endif
    if( m_openMode != readMode )
    {
        // FIXME: Throw an error.
        return 0;
    }

#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Reading " << len << " bytes from " << m_filename;
    LOG_DEBUG( stream.str() );
    }
#endif
    char* dataPtr( data );
    int lenRemaining( len );
    while( lenRemaining > 0 )
    {
        // Read from current page.
        int readFromPage( 0 );
        int pageRemaining( m_currentPageDataSize - m_currentPageDataOffset );
        if( lenRemaining < pageRemaining )
        {
            readFromPage = lenRemaining;
        }
        else
        {
            readFromPage = pageRemaining;
        }

#ifdef KIAMA_DEBUG
        {
        LiteStream stream;
        stream << "Reading " << readFromPage << " bytes from current page";
        LOG_DEBUG( stream.str() );
        }
#endif

        int offset( 0 );
        offset = m_currentSector * m_fs.m_memory.sectorSize();
        offset += ( 2 + m_currentPageInSector ) * 256;
        offset += 1 + m_currentPageDataOffset;
        m_fs.m_memory.read( offset, dataPtr, readFromPage );
        m_currentPageDataOffset += readFromPage;
        lenRemaining -= readFromPage;
        dataPtr += readFromPage;

        // FIXME: Don't scan if at EOF.
        if( lenRemaining > 0 )
        {
            // Find next page
#ifdef KIAMA_DEBUG
            LOG_DEBUG( "Looking for next page" );
#endif

            if( !findPage( m_currentPageSeq + 1 ) )
            {
#ifdef KIAMA_DEBUG
                LOG_DEBUG( "EOF" );
#endif
                break;
            }
        }
    }

    return len - lenRemaining;
}

int KiamaFS::File::write( const char* data, int len )
{
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Write length " << len;
    LOG_DEBUG( stream.str() );
    }
#endif
    if( m_openMode != writeMode )
    {
        m_fs.m_error = true;
        return 0;
    }

    bool fsFull( false );
    if( m_currentPageDataOffset == 255 )
    {
        finalisePage( m_currentPageDataOffset, false );
        int sectorBeforeAllocate( m_currentSector );
        if( findFreePage() )
        {
            ++m_currentPageSeq;
            allocatePage();

            if( m_currentSector != sectorBeforeAllocate )
            {
                m_newSectors.push_back( m_currentSector );
            }
        }
        else
        {
#ifdef KIAMA_DEBUG
            LOG_DEBUG( "Unable to find free page" );
#endif
            m_fs.m_error = true;
            fsFull = true;
        }
    }

    int lenRemain( len );
    while( lenRemain > 0 && !fsFull )
    {
        int writeToPage( 0 );
        int pageRemain( 255 - m_currentPageDataOffset );
        if( lenRemain < pageRemain )
        {
            writeToPage = lenRemain;
        }
        else
        {
            writeToPage = pageRemain;
        }
        
        int offset( 0 );
        offset = m_currentSector * m_fs.m_memory.sectorSize();
        offset += ( 2 + m_currentPageInSector ) * 256;
        offset += 1 + m_currentPageDataOffset;

        int inOffset( len - lenRemain );

        m_fs.m_memory.write( offset, data + inOffset, writeToPage );

        m_newSize += writeToPage;

        lenRemain -= writeToPage;
        m_currentPageDataOffset += writeToPage;

        pageRemain = 255 - m_currentPageDataOffset;
        if( pageRemain == 0 && lenRemain > 0 )
        {
            finalisePage( m_currentPageDataOffset, false );
            int sectorBeforeAllocate( m_currentSector );
            if( findFreePage() )
            {
                ++m_currentPageSeq;
                allocatePage();

                if( m_currentSector != sectorBeforeAllocate )
            {
                m_newSectors.push_back( m_currentSector );
            }
            }
            else
            {
#ifdef KIAMA_DEBUG
                LOG_DEBUG( "Unable to find free page" );
#endif
                m_fs.m_error = true;
                fsFull = true;
            }
        }
    }

    return ( len - lenRemain );
}

bool KiamaFS::File::error()
{
    return( m_fs.m_error || m_fs.m_memory.error() );
}

void KiamaFS::File::commit()
{
#ifdef KIAMA_DEBUG
    LOG_DEBUG( "Commit" );
#endif
    if( m_openMode != writeMode )
    {
        return;
    }

    finalisePage( m_currentPageDataOffset, true );

    markPagesForDeletion( m_filename, m_version, m_sectors );

    m_openMode = notOpen;

    IndexEntry indexEntry;
    indexEntry.m_version = m_newVersion;
    indexEntry.m_sectors = m_newSectors;
    indexEntry.m_size = m_newSize;
    m_fs.m_index[m_filename] = indexEntry;

    //////////////////////////////////////////////
    Map< String, IndexEntry >::const_iterator debug2It;
#ifdef KIAMA_DEBUG
    LOG_DEBUG( "New index:\r\n" );
    for( debug2It = m_fs.m_index.begin(); debug2It != m_fs.m_index.end(); ++debug2It )
    {
        LiteStream stream;
        stream << debug2It->first << ": Ver " << debug2It->second.m_version << " Sz " << debug2It->second.m_size << " Sect ";
        Vector< unsigned short >::const_iterator debug3It( debug2It->second.m_sectors.begin() );
        for( ; debug3It != debug2It->second.m_sectors.end(); ++debug3It )
        {
            stream << *debug3It << " ";
        }
        LOG_DEBUG( stream.str() );
    }
#endif
}

void KiamaFS::File::erase()
{
#ifdef KIAMA_DEBUG
    LOG_DEBUG( "Deleting " + m_filename );
#endif

    if( m_openMode == notOpen )
    {
        // Need to get current version sector list.
        open( readMode );
    }

    markPagesForDeletion( m_filename, m_version, m_sectors );

    Map< String, IndexEntry >::iterator indexEntryIt( m_fs.m_index.find( m_filename ) );
    if( indexEntryIt != m_fs.m_index.end() )
    {
        m_fs.m_index.erase( indexEntryIt );
    }
}

void KiamaFS::File::getSectors()
{
    Map< String, IndexEntry >::iterator indexEntryIt( m_fs.m_index.find( m_filename ) );
    if( indexEntryIt != m_fs.m_index.end() )
    {
        IndexEntry& indexEntry( indexEntryIt->second );
        m_version = indexEntry.m_version;
        m_sectors = indexEntry.m_sectors;
        m_size = indexEntry.m_size;
    }
}

void KiamaFS::File::allocatePage()
{
    char rawEntry[28];
    rawEntry[0] = '\x55';
    std::strncpy( &rawEntry[1], m_filename.c_str(), 24 );

    rawEntry[25] = m_newVersion;
    rawEntry[26] = m_currentPageSeq & 0xFF;
    rawEntry[27] = ( m_currentPageSeq >> 8 ) & 0xFF;

#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Allocate page " << m_currentPageInSector << " in sector " << m_currentSector;
    LOG_DEBUG( stream.str() );
    }
#endif
    int offset( 0 );
    offset = m_currentSector * m_fs.m_memory.sectorSize();
    offset += m_currentPageInSector * 32;
    m_fs.m_memory.write( offset, rawEntry, 28 );

    m_currentSectorHeader.m_entries[m_currentPageInSector].m_free = 0;

    // Don't write end marker - only when page is finalised
    // to guarantee the page data are valid.
}

void KiamaFS::File::finalisePage( int dataBytesWritten, bool lastPage )
{
    int offset( 0 );
    offset = m_currentSector * m_fs.m_memory.sectorSize();
    offset += ( 2 + m_currentPageInSector ) * 256;

    // FIXME: Ick! I should look at the signedness and size of all
    // variables in this thing!
    const char sz( dataBytesWritten );
    m_fs.m_memory.write( offset, &sz, 1 );

    // If this is the last page, mark that in the sector header entry.
    offset = 0;
    offset = m_currentSector * m_fs.m_memory.sectorSize();
    offset += m_currentPageInSector * 32;
    offset += 28;

    char marker( '\x55' );
    if( lastPage )
    {
        m_fs.m_memory.write( offset, &marker, 1 ); // Last page mark.
    }

    offset += 3;
    m_fs.m_memory.write( offset, &marker, 1 ); // Entry valid mark.
}

void KiamaFS::File::markPagesForDeletion( const String& filename, int version, Vector< unsigned short >& sectors )
{
    Vector< unsigned short >::const_iterator sectorIt;
    for( sectorIt = sectors.begin(); sectorIt != sectors.end(); ++sectorIt )
    {
        SectorHeader header( m_fs.readSectorHeader( *sectorIt ) );

        int currentPageInSector( 0 );
        Vector< SectorHeaderEntry >::const_iterator entryIt;
        for( entryIt = header.m_entries.begin(); entryIt != header.m_entries.end(); ++entryIt )
        {
#ifdef KIAMA_DEBUG
            if( entryIt->m_valid &&
                entryIt->m_delete &&
                entryIt->m_filename == filename &&
                entryIt->m_version == version )
            {
                LOG_DEBUG( "Found page already deleted!" );
            }
#endif

            if( entryIt->m_valid &&
                !entryIt->m_delete &&
                entryIt->m_filename == filename &&
                entryIt->m_version == version )
            {
                int offset( 0 );
                offset = *sectorIt * m_fs.m_memory.sectorSize();
                offset += currentPageInSector * 32;
                offset += 30;

#ifdef KIAMA_DEBUG
                {
                LiteStream stream;
                stream << "Marking page "
                       << currentPageInSector
                       << " in sector "
                       << *sectorIt
                       << " for deletion";
                LOG_DEBUG( stream.str() );
                }
#endif

                char marker( '\x55' );
                m_fs.m_memory.write( offset, &marker, 1 );
            }

            ++currentPageInSector;
        }
    }
}

bool KiamaFS::File::findPage( int pageSeq )
{
    // Work out which sector contains the specified page for this file.
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Looking for page " << pageSeq;
    LOG_DEBUG( stream.str() );
    }
#endif

    bool foundPage( false );
    if( m_currentSector != -1 )
    {
        if( findPageInSector( pageSeq, m_currentSector ) )
        {
            foundPage = true;
        }
    }

    if( !foundPage )
    {    
        Vector< unsigned short >::const_iterator it;
        for( it = m_sectors.begin(); it != m_sectors.end(); ++it )
        {
            if( findPageInSector( pageSeq, *it ) )
            {
                foundPage = true;
                break;
            }
        }
    }

    return foundPage;
}

bool KiamaFS::File::findPageInSector( int pageSeq, int sector )
{
#ifdef KIAMA_DEBUG
    {
    LiteStream stream;
    stream << "Scanning sector header for sector " << sector;
    stream << " Filename: " << m_filename << " Version: " << m_version << " Seq: " << pageSeq;
    LOG_DEBUG( stream.str() );
    }
#endif

    if( sector != m_currentSector)
    {
        m_currentSectorHeader = m_fs.readSectorHeader( sector );
    }

    bool foundPage( false );
    Vector< SectorHeaderEntry >::const_iterator it;
    int currentPageInSector( 0 );
    for( it = m_currentSectorHeader.m_entries.begin(); it != m_currentSectorHeader.m_entries.end(); ++it )
    {
#ifdef KIAMA_DEBUG
        {
        LiteStream stream;
        stream << "Valid: " << it->m_valid << " Delete: " << it->m_delete << " Filename: " << it->m_filename << " Version: " << it->m_version << " Seq: " << it->m_pageSeq;
        LOG_DEBUG( stream.str() );
        }
#endif
        if( it->m_valid &&
            !it->m_delete &&
            it->m_filename == m_filename &&
            it->m_version == m_version &&
            it->m_pageSeq == pageSeq )
        {
#ifdef KIAMA_DEBUG
            {
            LiteStream stream;
            stream << "Found page " << pageSeq << " in page " << currentPageInSector << " of sector " << sector;
            LOG_DEBUG( stream.str() );
            }
#endif
            m_currentSector = sector;
            m_currentPageSeq = it->m_pageSeq;
            m_currentPageInSector = currentPageInSector;

            int offset( 0 );
            offset = m_currentSector * m_fs.m_memory.sectorSize();
            offset += ( 2 + m_currentPageInSector ) * 256;
            char sz( 0 );
            m_fs.m_memory.read( offset, &sz, 1 );
            m_currentPageDataSize = static_cast< unsigned char >( sz );

            m_currentPageDataOffset = 0;

            foundPage = true;
            break;
        }
        ++currentPageInSector;
    }

    return foundPage;
}

bool KiamaFS::File::findFreePage()
{
    int startSector( 0 );
    if( m_fs.m_freeSector != -1 )
    {
        startSector = m_fs.m_freeSector;
    }

    bool foundFree( false );
    int sector( startSector );
    while( !foundFree )
    {
#ifdef KIAMA_DEBUG
        {
        LiteStream stream;
        stream << "Reading sector " << sector;
        LOG_DEBUG( stream.str() );
        }
#endif
        
        if( m_currentSector == -1 || sector != m_currentSector )
        {
            m_currentSectorHeader = m_fs.readSectorHeader( sector );
        }

        Vector< SectorHeaderEntry >::const_iterator it;
        int currentPageInSector( 0 );
        for( it = m_currentSectorHeader.m_entries.begin(); it != m_currentSectorHeader.m_entries.end(); ++it )
        {
#ifdef KIAMA_DEBUG
            {
            LiteStream stream;
            stream << "Considering page " << currentPageInSector;
            LOG_DEBUG( stream.str() );
            }
#endif
            if( it->m_free )
            {
#ifdef KIAMA_DEBUG
                {
                LiteStream stream;
                stream << "Page " << currentPageInSector << " in sector " << sector << " is free.";
                LOG_DEBUG( stream.str() );
                }
#endif
                m_fs.m_freeSector = sector;
                m_currentSector = sector;
                m_currentPageInSector = currentPageInSector;
                m_currentPageDataSize = 0;
                m_currentPageDataOffset = 0;
                foundFree = true;
                break;
            }
            ++currentPageInSector;
        }

        ++sector;
        if( sector >= ( m_fs.m_memory.size() / m_fs.m_memory.sectorSize() ) )
        {
            sector = 0;
        }
        if( sector == startSector )
        {
            break;
        }
    }

    return foundFree;
}

KiamaFS::KiamaFS( Memory& memory ) :
  m_freeSector( -1 ),
  m_memory( memory )
{
    assert( m_memory.type() == Memory::flash );
}

KiamaFS::File* KiamaFS::file( const String& filename )
{
    return new File( filename, *this );
}

void KiamaFS::purge()
{
    int currentSector( 0 );
    for( ; currentSector < ( m_memory.size() / m_memory.sectorSize() ); ++currentSector )
    {
        bool sectorUsed( false );
        bool canErase( true );
        SectorHeader header( readSectorHeader( currentSector ) );
        Vector< SectorHeaderEntry >::const_iterator it;
        for( it = header.m_entries.begin(); it != header.m_entries.end(); ++it )
        {
            if( !it->m_free )
            {
                // Something in the sector has been written to, even
                // if incompletely.
                sectorUsed = true;
            }

            if( it->m_valid && !it->m_delete )
            {
                // We have a validly allocated page that has not been
                // marked for deletion yet.
                canErase = false;
                break;
            }
        }

        if( sectorUsed && canErase )
        {
#ifdef KIAMA_DEBUG
            {
            LiteStream stream;
            stream << "Purge sector " << currentSector;
            LOG_DEBUG( stream.str() );
            }
#endif
            m_memory.erase( currentSector * m_memory.sectorSize(), m_memory.sectorSize() );
        }
    }

    m_error = false; // Clear error if caused by FS full.
}

KiamaFS::SectorHeader KiamaFS::readSectorHeader( int sectorAddr )
{
    // Check start and end marker for each entry. Don't add to the entries
    // vector if corrupt.

    //std::cerr << "Reading header for sector " << sectorAddr << std::endl;
    SectorHeader header;

    char rawEntry[32];
    int offset( 0 );
    offset = sectorAddr * m_memory.sectorSize();
    bool sectorEmpty( false );
    for( int i = 0; i < 14; ++i )
    {
        if( !sectorEmpty )
        {
            m_memory.read( offset, rawEntry, 32 );

            //std::cerr << "Reading sector header entry " << i << std::endl;
            SectorHeaderEntry entry;
            entry.m_valid = ( rawEntry[0] == '\x55' && rawEntry[31] == '\x55' );
            entry.m_free = ( rawEntry[0] == '\xFF' );
            entry.m_delete = ( rawEntry[30] == '\x55' );
            if( entry.m_valid )
            {
                char rawFilename[25];
                ::memcpy( &rawFilename, &rawEntry[1], 24 );
                rawFilename[24] = '\0';
                entry.m_filename = String( rawFilename );
                entry.m_version = *( (unsigned char*)&rawEntry[25] );
                entry.m_pageSeq = rawEntry[26];
                entry.m_pageSeq += ( rawEntry[27] << 8 );
                entry.m_lastPage = ( rawEntry[28] == '\x55' );
                /*
                std::cerr << "Entry seq: " << i <<
                                " nm: " << entry.m_filename <<
                                " v: " << entry.m_version <<
                                " n: " << entry.m_pageNum <<
                                " t: " << entry.m_totalPages << std::endl;
                */
            }

            header.m_entries.push_back( entry );

            if( ( i == 0 ) && ( rawEntry[0] == '\xFF' ) )
            {
                // Don't bother to read any more headers.
                sectorEmpty = true;
            }
        }
        else
        {
            // The sector is empty. Set all entries to reflect this.
            SectorHeaderEntry entry;
            entry.m_valid = false;
            entry.m_free = true;
            entry.m_delete = false;
            header.m_entries.push_back( entry );
        }

        offset += 32;
    }

    return header;
}

void KiamaFS::createIndex()
{
    m_index.clear();

    Map< String, int > latestCompleteVersions;

    // Pass 1: Find latest complete versions for all files, by searching for
    // non-deleted last page header entries with highest version number.
    int currentSector( 0 );
    for( ; currentSector < ( m_memory.size() / m_memory.sectorSize() ); ++currentSector )
    {
        SectorHeader header( readSectorHeader( currentSector ) );
        Vector< SectorHeaderEntry >::const_iterator it;
        int currentPageInSector( 0 );
        for( it = header.m_entries.begin(); it != header.m_entries.end(); ++it )
        {
            if( it->m_valid && !it->m_delete && it->m_lastPage )
            {
                if( latestCompleteVersions.find( it->m_filename ) != latestCompleteVersions.end() )
                {
                    if( latestCompleteVersions[ it->m_filename ] < it->m_version )
                    {
                        latestCompleteVersions[ it->m_filename ] = it->m_version;
                    }
                }
                else
                {
                    latestCompleteVersions[ it->m_filename ] = it->m_version;
                }
            }

            ++currentPageInSector;
        }
    }

    // DEBUG
#ifdef KIAMA_DEBUG
    LOG_DEBUG( "Index first pass:" );
    Map< String, int >::const_iterator debugIt;
    for( debugIt = latestCompleteVersions.begin(); debugIt != latestCompleteVersions.end(); ++debugIt )
    {
        LiteStream stream;
        stream << debugIt->first << " " << debugIt->second;
        LOG_DEBUG( stream.str() );
    }
#endif

    // Pass 2: Count bytes in all pages for latest versions, to find total size,
    // by searching for all header entries belonging to latest version and
    // counting bytes in each page.
    currentSector = 0;
    for( ; currentSector < ( m_memory.size() / m_memory.sectorSize() ); ++currentSector )
    {
        //std::cerr << "Considering sector " << currentSector << std::endl;
        SectorHeader header( readSectorHeader( currentSector ) );
        Vector< SectorHeaderEntry >::const_iterator it;
        int currentPageInSector( 0 );
        for( it = header.m_entries.begin(); it != header.m_entries.end(); ++it )
        {
            //std::cerr << "Considering page " << currentPageInSector << std::endl;
            if( it->m_valid && !it->m_delete )
            {
                //std::cerr << "valid not deleted" << std::endl;
                bool doMarkDeleted( false );
                if( latestCompleteVersions.find( it->m_filename ) != latestCompleteVersions.end() )
                {
                    //std::cerr << "lcv found" << std::endl;
                    if( it->m_version == latestCompleteVersions[ it->m_filename ] )
                    {
                        //std::cerr << "lcv match" << std::endl;
                        int offset( 0 );
                        offset = currentSector * m_memory.sectorSize();
                        offset += ( 2 + currentPageInSector ) * 256;
                        char sz( 0 );
                        m_memory.read( offset, &sz, 1 );

                        Map< String, IndexEntry >::iterator indexIt( m_index.find( it->m_filename ) );
                        if( indexIt != m_index.end() )
                        {
                            IndexEntry& indexEntry( indexIt->second );
                            //std::cerr << "In index with size " << indexEntry.m_size << std::endl;
                            Vector< unsigned short >::const_iterator indexSectorsIt( indexEntry.m_sectors.begin() );
                            for( ; indexSectorsIt != indexEntry.m_sectors.end(); ++indexSectorsIt )
                            {
                                if( *indexSectorsIt == currentSector )
                                {
                                    break;
                                }
                            }

                            if( indexSectorsIt == indexEntry.m_sectors.end() )
                            {
                                indexEntry.m_sectors.push_back( currentSector );
                            }

                            indexEntry.m_size += static_cast< unsigned char >( sz );
                        }
                        else
                        {
                            //std::cerr << "Not in index" << std::endl;
                            IndexEntry indexEntry;
                            indexEntry.m_version = it->m_version;
                            indexEntry.m_sectors.push_back( currentSector );
                            indexEntry.m_size = static_cast< unsigned char >( sz );
                            m_index[it->m_filename] = indexEntry;
                        }
                    }
                    else
                    {
                        // Pages for old version. Mark for deletion.
                        doMarkDeleted = true;
                    }
                }
                else
                {
                    // Only version is incomplete (final page could not be found
                    // therefore not in index). Mark for deletion.
                    doMarkDeleted = true;
                }

                if( doMarkDeleted )
                {
                    //std::cerr << "Old or incomplete pages. Marking for deletion." << std::endl;
                    int offset( 0 );
                    offset = currentSector * m_memory.sectorSize();
                    offset += currentPageInSector * 32;
                    offset += 30;
                    char marker( '\x55' );
                    m_memory.write( offset, &marker, 1 );
                }
            }

            ++currentPageInSector;
        }
    }

    // DEBUG
#ifdef KIAMA_DEBUG
    Map< String, IndexEntry >::const_iterator debug2It;
    LOG_DEBUG( "Index second pass:" );
    for( debug2It = m_index.begin(); debug2It != m_index.end(); ++debug2It )
    {
        LiteStream stream;
        stream << debug2It->first << ": Ver " << debug2It->second.m_version << " Sz " << debug2It->second.m_size << " Sect ";
        Vector< unsigned short >::const_iterator debug3It( debug2It->second.m_sectors.begin() );
        for( ; debug3It != debug2It->second.m_sectors.end(); ++debug3It )
        {
            stream << *debug3It << " ";
        }
        LOG_DEBUG( stream.str() );
    }
#endif
}

const Map< String, KiamaFS::IndexEntry >& KiamaFS::getIndex()
{
    return m_index;
}

} // namespace Agape
