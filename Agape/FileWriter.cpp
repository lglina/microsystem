#include "Loggers/Logger.h"
#include "FileWriter.h"
#include "String.h"

#include <fstream>

namespace Agape
{

FileWriter::FileWriter( const String& filename,
                        enum OpenMode openMode )
{
    std::ios_base::openmode iosOpenMode;
    if( openMode == modeRead )
    {
        iosOpenMode = std::ios::in | std::ios::binary;
    }
    else
    {
        iosOpenMode = std::ios::out | std::ios::trunc | std::ios::binary;
    }
    
    m_file = std::fstream( filename.c_str(), iosOpenMode );
}

FileWriter::~FileWriter()
{
    m_file.close();
}

bool FileWriter::isOpen()
{
    return m_file.is_open();
}

void FileWriter::close()
{
    m_file.close();
}

int FileWriter::read( char* data, int len )
{
    // FIXME: No error handling.
    m_file.read( data, len );
    return len;
}

int FileWriter::write( const char* data, int len )
{
    // FIXME: No error handling.
    m_file.write( data, len );
    return len;
}

bool FileWriter::error()
{
    // FIXME: No error handling.
    return false;
}

} // namespace Agape
