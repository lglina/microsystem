#ifndef AGAPE_CARLO_PROGRAM_MANAGER_H
#define AGAPE_CARLO_PROGRAM_MANAGER_H

#include "Collections.h"
#include "ExecutionContext.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

namespace World
{
class Coordinates;
} // namespace World

class AssetLoader;

using namespace Linda2;

namespace Carlo
{

class FunctionDispatcher;
class Linda2;

class ProgramManager
{
public:
    ProgramManager( TupleRouter& tupleRouter, FunctionDispatcher& functionDispatcher );
    ~ProgramManager();

    // Load or reload.
    bool load( AssetLoaders::Factory& assetLoaderFactory, const World::Coordinates& coordinates, const String& name, const String& templateName = String() );
    bool unload( const String& name );

    // Erase the program file. Note - this deliberately doesn't perform an
    // unload(). The program could be unload()'ed first, or can safely be
    // left in memory even after it's erased from the asset store.
    bool erase( AssetLoaders::Factory& assetLoaderFactory, const World::Coordinates& coordinates, const String& name );

    // Determine if a currently loaded program is based on a template.
    bool isFromTemplate( const String& name, String& templateName ) const;

    // Determine if a named program is currently loaded.
    bool isLoaded( const String& name ) const;

    // Return runtime errors from the last tuple handled by each actor in
    // this program.
    Vector< ExecutionContext::RuntimeError > runtimeErrors( const String& name );

private:
    struct LoadedProgram
    {
        String m_name;
        String m_templateName;
        Linda2* m_program;
    };

    int readLine( AssetLoader* assetLoader, String& line, int& offset );

    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;

    Vector< struct LoadedProgram > m_programs;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_PROGRAM_MANAGER_H
