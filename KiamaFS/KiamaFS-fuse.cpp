#define FUSE_USE_VERSION 39

#include "Collections.h"
#include "KiamaFS.h"
#include "Memories/FileMemory.h"
#include "String.h"

#include <fuse.h>
#include <cstring>
#include <cstddef>
#include <unistd.h>
#include <sys/types.h>

#include <iostream>
#include <string>

#include <libgen.h>

namespace
{
    struct options
    {
        const char* filename;
    } options;

    #define OPTION( t, p ) \
        { t, offsetof( struct options, p ), 1 }

    struct fuse_opt option_spec[] =
    {
        OPTION( "--filename=%s", filename ),
        FUSE_OPT_END
    };

	Agape::Memories::File* fileMemory;
    Agape::KiamaFS* fs;

    struct OpenFile
    {
        OpenFile( const Agape::String& name, Agape::KiamaFS* fs ) :
          m_name( name ),
          m_file( name, *fs )
        {
        }

        Agape::String m_name;
        Agape::KiamaFS::File m_file;
    };

    Agape::Vector< OpenFile* > openFiles;
} // anonymous namespace

static int kiama_getattr(const char *path, struct stat *stbuf, struct fuse_file_info* fi )
{
    std::cerr << "Call to getattr for " << basename( (char*)path ) << std::endl;
    const Agape::Map< Agape::String, int >& index( fs->getIndex() );

    Agape::Map< Agape::String, int >::const_iterator it( index.find( basename( (char*)path ) ) );
    if( it != index.end() )
    {
        std::cout << "Found in index with size " << it->second << std::endl;
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = it->second;
    }
    else if( strcmp( path, "/" ) == 0 )
    {
        std::cout << "Root directory" << std::endl;
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else
    {
        auto it = openFiles.begin();
        for( ; it != openFiles.end(); ++it )
        {
            if( ( *it )->m_name == basename( (char*)path ) )
            {
                // Stat on new file? Return zero size.
                stbuf->st_mode = S_IFREG | 0644;
                stbuf->st_nlink = 1;
                stbuf->st_size = 0;
                break;
            }
        }

        if( it == openFiles.end() )
        {
            std::cout << "File not found." << std::endl;
            return -ENOENT;
        }
    }

    stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int kiama_unlink( const char* path )
{
    const Agape::Map< Agape::String, int >& index( fs->getIndex() );

    if( index.find( basename( (char*)path ) ) != index.end() )
    {
        fs->erase( basename( (char*) path ) );
    }
    else
    {
        return -ENOENT;
    }

    for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
    {
        if( ( *it )->m_name == basename( (char*)path ) )
        {
            delete( *it );
            openFiles.erase( it );
            break;
        }
    }

    return 0;
}

static int kiama_readdir( const char* path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi, fuse_readdir_flags )
{
    const Agape::Map< Agape::String, int >& index( fs->getIndex() );

    filler( buffer, ".", NULL, 0, 0 );
    filler( buffer, "..", NULL, 0, 0 );

    if( strcmp( path, "/" ) == 0 )
    {
        for( auto it = index.begin(); it != index.end(); ++it )
        {
            filler( buffer, it->first.c_str(), NULL, 0, 0 );
        }
    }
    else
    {
        return -ENOENT;
    }
    
    return 0;
}

static int kiama_truncate(const char *path, off_t size, struct fuse_file_info* fi )
{
	// Not required.
    return 0;
}

static int kiama_open(const char *path, struct fuse_file_info *fi)
{
    const Agape::Map< Agape::String, int >& index( fs->getIndex() );

    if( index.find( basename( (char*)path ) ) != index.end() )
    {
        for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
        {
            if( ( *it )->m_name == basename( (char*)path ) )
            {
                // Attempted simultaneous access?
                std::cout << "Open: " << path << " already in open files list" << std::endl;
                return -EACCES;
            }
        }

        if( ( fi->flags & O_ACCMODE ) == O_RDONLY )
        {
            openFiles.push_back( new OpenFile( basename( (char*)path ), fs ) );
            openFiles.back()->m_file.open( Agape::KiamaFS::File::readMode );
        }
        else if( ( fi->flags & O_ACCMODE ) == O_WRONLY )
        {
            openFiles.push_back( new OpenFile( basename( (char*)path ), fs ) );
            openFiles.back()->m_file.open( Agape::KiamaFS::File::writeMode );
        }
        else
        {
            // O_RW? Unsupported.
            std::cout << "Open: " << path << " unsupported mode" << std::endl;
            return -EACCES;
        }
    }
    else
    {
        return -ENOENT;
    }
    
    return 0;
}

static int kiama_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
    {
        if( ( *it )->m_name == basename( (char*)path ) )
        {
            if( ( *it )->m_file.seek( offset ) )
            {
                return ( *it )->m_file.read( buf, size );
            }
            else
            {
                return 0;
            }
        }
    }

    return -ENOENT;
}

static int kiama_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
    for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
    {
        if( ( *it )->m_name == basename( (char*)path ) )
        {
            // DANGER: Assumes writes are sequential. Offset ignored.
            return ( *it )->m_file.write( buf, size );
        }
    }

    return -ENOENT;
}

static int kiama_create( const char* path, mode_t mode, struct fuse_file_info* fi )
{
    const Agape::Map< Agape::String, int >& index( fs->getIndex() );

    if( index.find( basename( (char*)path ) ) != index.end() )
    {
        // Already exists.
        return -EACCES;
    }
    else
    {
        for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
        {
            if( ( *it )->m_name == basename( (char*)path ) )
            {
                // Attempted simultaneous creation?
                return -EACCES;
            }
        }

        if( ( fi->flags & O_ACCMODE ) == O_RDONLY )
        {
            openFiles.push_back( new OpenFile( basename( (char*)path ), fs ) );
            openFiles.back()->m_file.open( Agape::KiamaFS::File::readMode );
        }
        else if( ( fi->flags & O_ACCMODE ) == O_WRONLY )
        {
            openFiles.push_back( new OpenFile( basename( (char*)path ), fs ) );
            openFiles.back()->m_file.open( Agape::KiamaFS::File::writeMode );
        }
        else
        {
            // O_RW? Unsupported.
            return -EACCES;
        }
    }
    
    return 0;
}

static int kiama_release( const char* path, struct fuse_file_info* fi )
{
    for( auto it = openFiles.begin(); it != openFiles.end(); ++it )
    {
        if( ( *it )->m_name == basename( (char*)path ) )
        {
            ( *it )->m_file.commit();
            delete( *it );
            openFiles.erase( it );
            return 0;
        }
    }

    return -ENOENT;
}

static const struct fuse_operations kiama_oper = {
	.getattr	= kiama_getattr,
    .unlink     = kiama_unlink,
	.truncate	= kiama_truncate,
	.open		= kiama_open,
	.read		= kiama_read,
	.write		= kiama_write,
    .release    = kiama_release,
    .readdir    = kiama_readdir,
    .create     = kiama_create
};

int main( int argc, char** argv )
{
    struct fuse_args args = FUSE_ARGS_INIT( argc, argv );

    options.filename = ::strdup( "filememory.dat" );

    if( fuse_opt_parse( &args, &options, option_spec, nullptr ) == -1 )
    {
        return 1;
    }

    fileMemory = new Agape::Memories::File( options.filename, Agape::Memory::flash );
    fs = new Agape::KiamaFS( *fileMemory );

    int ret( fuse_main( args.argc, args.argv, &kiama_oper, nullptr ) );

    fuse_opt_free_args( &args );
    return ret;
}
