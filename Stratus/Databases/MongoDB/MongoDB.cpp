#include "Loggers/Logger.h"
#include "MongoDB.h"

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

#include <string>
#include <fstream>

namespace Agape
{

namespace Databases
{

namespace MongoDB
{

mongocxx::instance* MongoDB::s_instance( nullptr );
mongocxx::pool* MongoDB::s_pool( nullptr );

mongocxx::instance& MongoDB::instance()
{
    if( !s_instance )
    {
        s_instance = new mongocxx::instance();
    }

    return *s_instance;
}

mongocxx::pool& MongoDB::pool()
{
    if( !s_pool )
    {
        std::string mongoURI;
        std::ifstream secretFile( "mongodb-secret" );
        std::getline( secretFile, mongoURI );

        if( !secretFile.is_open() || secretFile.fail() )
        {
            LOG_DEBUG( "ERROR: Failed to read MongoDB URI from 'mongodb-secret'. Please save your MongoDB URI (including username and password) into that file and run again." );
            exit( 1 );
        }

        secretFile.close();

        s_pool = new mongocxx::pool(
            mongocxx::uri (
                mongoURI
            )
        );
    }

    return *s_pool;
}

} // namespace MongoDB

} // namespace Databases

} // namespace Agape
