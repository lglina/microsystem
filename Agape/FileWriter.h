#ifndef AGAPE_FILE_WRITER_H
#define AGAPE_FILE_WRITER_H

#include "ReadableWritable.h"

#include <fstream>

namespace Agape
{

class String;

class FileWriter : public ReadableWritable
{
public:
    enum OpenMode
    {
        modeRead,
        modeWrite
    };

    FileWriter( const String& filename,
                enum OpenMode openMode );
    ~FileWriter();

    bool isOpen();
    void close();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

private:
    std::fstream m_file;
};

} // namespace Agape

#endif // AGAPE_FILE_WRITER_H
