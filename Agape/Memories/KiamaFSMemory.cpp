#include "KiamaFS.h"
#include "KiamaFSMemory.h"
#include "String.h"

namespace Agape
{

namespace Memories
{

KiamaFS::KiamaFS( Agape::KiamaFS& fs, const String& filename ) :
  m_fs( fs ),
  m_filename( filename ),
  m_isOpen( false ),
  m_writeMode( false ),
  m_file( nullptr )
{
}

enum Memory::Type KiamaFS::type()
{
    return eeprom;
}

int KiamaFS::read( int addr, char* data, int len )
{
    if( m_file == nullptr )
    {
        m_file = m_fs.file( m_filename );
        m_isOpen = m_file->open( Agape::KiamaFS::File::readMode );
        m_writeMode = false;
    }

    if( m_isOpen )
    {
        return( m_file->read( data, len ) );
    }
    
    // Dummy read.
    *data = '\xFF';
    return 1;
}

int KiamaFS::write( int addr, const char* data, int len )
{
    if( m_file == nullptr )
    {
        m_file = m_fs.file( m_filename );
        m_isOpen = m_file->open( Agape::KiamaFS::File::writeMode );
        m_writeMode = true;
    }

    if( m_isOpen )
    {
        int written( m_file->write( data, len ) );
        return written;
    }

    return 0;
}

bool KiamaFS::erase( int addr, int len )
{
    return false;
}

void KiamaFS::seek( int offset )
{
    if( m_file != nullptr )
    {
        delete( m_file );
        m_file = nullptr;
        m_isOpen = false;
        m_writeMode = false;
    }
}

int KiamaFS::size()
{
    if( ( m_file != nullptr ) && m_isOpen )
    {
        return m_file->size();
    }

    return 0;
}

int KiamaFS::sectorSize()
{
    return -1;
}

void KiamaFS::flushOutput()
{
    if( m_writeMode )
    {
        m_file->commit();
    }
}

} // namespace Memories

} // namespace Agape
