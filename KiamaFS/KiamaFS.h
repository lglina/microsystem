#ifndef AGAPE_KIAMA_FS_H
#define AGAPE_KIAMA_FS_H

#include "Collections.h"
#include "ReadableWritable.h"
#include "String.h"

namespace Agape
{

class Memory;

class KiamaFS
{
public:
    class SectorHeaderEntry
    {
    public:
        bool m_valid;      // 1B start marker
        bool m_free;       // ..
        String m_filename; // 24B filename
        int m_version;     // 1B
        int m_pageSeq;     // 2B, zero-based.
        bool m_lastPage;   // 1B
                           // 1B reserved.
        bool m_delete;     // 1B
                           // 1B end marker
    };

    class SectorHeader
    {
    public:
        Vector< SectorHeaderEntry > m_entries; // 512B (last two entries unusable)
    };

    // Each page contains 1B used byte count, then data.

    class File : public ReadableWritable
    {
    public:
        enum OpenMode
        {
            notOpen,
            readMode,
            writeMode
        };

        File( const String& filename, KiamaFS& fs );

        bool open( OpenMode openMode );
        int size();
        bool seek( int offset );
        virtual int read( char* data, int len );
        virtual int write( const char* data, int len );
        virtual bool error();
        void commit();
        void erase();

    private:
        void getSectors();
        void allocatePage();
        void finalisePage( int dataBytesWritten, bool lastPage );
        void markPagesForDeletion( const String& filename, int version, Vector< unsigned short >& sectors );
        bool findPage( int pageSeq );
        bool findPageInSector( int pageSeq, int sector );
        bool findFreePage();

        const String m_filename;

        KiamaFS& m_fs;

        OpenMode m_openMode;

        int m_version;
        int m_newVersion;
        Vector< unsigned short > m_sectors;
        int m_size;
        Vector< unsigned short > m_newSectors;
        int m_newSize;

        int m_currentSector;
        int m_currentPageSeq;
        int m_currentPageInSector;
        int m_currentPageDataSize;
        int m_currentPageDataOffset;
        SectorHeader m_currentSectorHeader;

        bool m_writing;
    };

    class IndexEntry
    {
    public:
        int m_version;
        Vector< unsigned short > m_sectors;
        int m_size;
    };

    KiamaFS( Memory& memory );

    File* file( const String& filename );

    void purge();

    void createIndex();
    const Map< String, IndexEntry >& getIndex();

private:
    SectorHeader readSectorHeader( int sectorAddr );

    int m_freeSector;

    Memory& m_memory;

    Map< String, IndexEntry > m_index;

    bool m_error;
};

} // namespace Agape

#endif // AGAPE_KIAMA_FS_H
