#ifndef AGAPE_DATABASES_MONGO_DB_H
#define AGAPE_DATABASES_MONGO_DB_H

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

namespace Agape
{

namespace Databases
{

namespace MongoDB
{

class MongoDB
{
public:
    static mongocxx::instance& instance();
    static mongocxx::pool& pool();

private:
    static mongocxx::instance* s_instance;
    static mongocxx::pool* s_pool;
};

} // namespace MongoDB

} // namespace Databases

} // namespace Agape

#endif // AGAPE_DATABASES_MONGO_DB_H
