#include "Actors/Actor.h"
#include "Actors/Linda2Actor.h"
#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "FunctionDispatcher.h"
#include "Lexer.h"
#include "Linda2.h"
#include "Parser.h"
#include "ProgramManager.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleHandler.h"
#include "TupleRouter.h"

namespace
{
    const int bufferSize( 256 );
} // Anonymous namespace

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

ProgramManager::ProgramManager( TupleRouter& tupleRouter, FunctionDispatcher& functionDispatcher ) :
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher )
{
}

ProgramManager::~ProgramManager()
{
    Vector< struct LoadedProgram >::iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        // Send destroy event to each actor.
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _Linda2 );
        TupleRouter::setTupleType( tuple, _Unload );
        Linda2* linda2( it->m_program );
        if( linda2 )
        {
            Vector< Actors::Linda2Actor* >::iterator actorIt( linda2->m_actors.begin() );
            for( ; actorIt != linda2->m_actors.end(); ++actorIt )
            {
                ( *actorIt )->accept( tuple );
            }
            delete( linda2 ); // Deregisters from dispatcher on destruction.
        }
    }
}

bool ProgramManager::load( AssetLoaders::Factory& assetLoaderFactory, const World::Coordinates& coordinates, const String& name, const String& templateName )
{
    String loadName;
    if( templateName.empty() )
    {
        loadName = name;
    }
    else
    {
        loadName = templateName;
    }

    bool opened( false );

    Vector< struct LoadedProgram >::iterator matchingTemplateIt( m_programs.end() );
    bool alreadyLoaded( false );
    Vector< struct LoadedProgram >::iterator programsIt( m_programs.begin() );
    for( ; programsIt != m_programs.end(); ++programsIt )
    {
        if( programsIt->m_name == name )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Program " + name + " already loaded." );
#endif
            alreadyLoaded = true;
        }

        if( !templateName.empty() &&
            ( programsIt->m_templateName == templateName ) &&
            programsIt->m_program )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Template " + templateName + " already loaded. Will re-use." );
#endif
            matchingTemplateIt = programsIt;
        }
    }

    if( !alreadyLoaded && ( matchingTemplateIt == m_programs.end() ) )
    {
        // Load the program.
#ifdef LOG_LOADERS
        LOG_DEBUG( "Loading program " + loadName + " with asset loader" );
#endif
        AssetLoader* loader( assetLoaderFactory.makeLoader( coordinates, loadName ) );
        if( loader->open() )
        {
            opened = true;

            struct LoadedProgram loadedProgram;
            loadedProgram.m_name = name;
            loadedProgram.m_program = nullptr;
            loadedProgram.m_templateName = templateName;

            Deque< Lexer::Token > tokens;
            Lexer lexer;

            int offset( 0 );
            String line;
            while( readLine( loader, line, offset ) != -1 )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "Lexing line" );
#endif
                lexer.lex( line, tokens );
            }

#ifdef LOG_LOADERS
            LOG_DEBUG( "Parsing" );
#endif
            Parser parser( tokens, m_tupleRouter, m_functionDispatcher, true );
            Linda2* linda2;
            if( parser.parse( linda2 ) )
            {
                loadedProgram.m_program = linda2;
                
                Vector< Actors::Linda2Actor* >& actors( linda2->m_actors );

                {
                Vector< Actors::Linda2Actor* >::const_iterator actorsIt( actors.begin() );
                for( ; actorsIt != actors.end(); ++actorsIt )
                {
                    Actors::Linda2Actor* actor( *actorsIt );

                    if( actorsIt == actors.begin() )
                    {
                        // First actor is always set to the program
                        // (not template) name, which will be the associated
                        // object snowflake.
                        actor->rename( name );
                    }

                    actor->doRegister();
                }
                }

                // Send create event to each actor.
#ifdef LOG_LOADERS
                LOG_DEBUG( "Sending Load tuples" );
#endif
                Tuple tuple;
                TupleRouter::setSourceActor( tuple, _Linda2 );
                TupleRouter::setTupleType( tuple, _Load );

                {
                Vector< Actors::Linda2Actor* >::iterator actorsIt( actors.begin() );
                for( ; actorsIt != actors.end(); ++actorsIt )
                {
                    ( *actorsIt )->accept( tuple );
                }
                }
            }
            // FIXME: Else indicate to the user that parsing failed...
            // Although they should see that if/when they edit the program.

            m_programs.push_back( loadedProgram );
        }

        delete( loader );
    }
    else if( !alreadyLoaded && ( matchingTemplateIt != m_programs.end() ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Cloning from already loaded template" );
#endif

        opened = true;

        // Make our own copies of the Actors within the previously loaded
        // program using the same template, but share pointers to
        // TupleHandlers (and reference count these) so we're not making
        // multiple copies of the ASTs.
        Linda2* linda2( new Linda2 );

        Vector< Actors::Linda2Actor* >& existingActors( matchingTemplateIt->m_program->m_actors );
        Vector< Actors::Linda2Actor* >::iterator existingActorsIt( existingActors.begin() );
        for( ; existingActorsIt != existingActors.end(); ++existingActorsIt )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Making copy of actor " + ( *existingActorsIt )->actorName() );
#endif
            Actors::Linda2Actor* actor( new Actors::Linda2Actor( **existingActorsIt ) );

            Vector< TupleHandler* >::iterator handlerIt( actor->m_tupleHandlers.begin() );
            for( ; handlerIt != actor->m_tupleHandlers.end(); ++handlerIt )
            {
                // Increment reference count to indicate that we've taken our
                // own copy of the pointer. Destruction of TupleHandlers is
                // performed in the Linda2Actor destructor only when the
                // reference count drops to zero.
                TupleHandler* handler( *handlerIt );
                ++handler->m_users;

#ifdef LOG_LOADERS
                LiteStream stream;
                stream << "Tuple handler for " << handler->m_tupleType
                       << " now has " << handler->m_users << " users.";
                LOG_DEBUG( stream.str() );
#endif
            }

            linda2->m_actors.push_back( actor );
        }

        struct LoadedProgram loadedProgram;
        loadedProgram.m_name = name;
        loadedProgram.m_program = linda2;
        loadedProgram.m_templateName = templateName;

        Vector< Actors::Linda2Actor* >& actors( linda2->m_actors );

        {
        Vector< Actors::Linda2Actor* >::iterator actorsIt( actors.begin() );
        for( ; actorsIt != actors.end(); ++actorsIt )
        {
            Actors::Linda2Actor* actor( *actorsIt );

            if( actorsIt == actors.begin() )
            {
                actor->rename( name );
            }

            actor->doRegister();
        }
        }

#ifdef LOG_LOADERS
        LOG_DEBUG( "Sending Load tuples" );
#endif
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _Linda2 );
        TupleRouter::setTupleType( tuple, _Load );

        {
        Vector< Actors::Linda2Actor* >::iterator actorsIt( actors.begin() );
        for( ; actorsIt != actors.end(); ++actorsIt )
        {
            ( *actorsIt )->accept( tuple );
        }
        }

        m_programs.push_back( loadedProgram );
    }
    else
    {
        opened = true; // Program already loaded.
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Program opened successfully" );
#endif

    return opened;
}

bool ProgramManager::unload( const String& name )
{
    Vector< struct LoadedProgram >::iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        if( it->m_name == name )
        {
            // Send destroy event to each actor.
            Tuple tuple;
            TupleRouter::setSourceActor( tuple, _Linda2 );
            TupleRouter::setTupleType( tuple, _Unload );
            Linda2* linda2( it->m_program );
            if( linda2 )
            {
                Vector< Actors::Linda2Actor* >::iterator actorIt( linda2->m_actors.begin() );
                for( ; actorIt != linda2->m_actors.end(); ++actorIt )
                {
                    ( *actorIt )->accept( tuple );
                }
                delete( it->m_program ); // Deregisters from dispatcher on destruction.
            }

            m_programs.erase( it );
            return true;
        }
    }

    return false;
}

bool ProgramManager::erase( AssetLoaders::Factory& assetLoaderFactory, const World::Coordinates& coordinates, const String& name )
{
    bool success( false );
    AssetLoader* loader( assetLoaderFactory.makeLoader( coordinates, name ) );
    success = loader->erase();

    delete( loader );

    return success;
}

bool ProgramManager::isFromTemplate( const String& name, String& templateName ) const
{
    Vector< struct LoadedProgram >::const_iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        if( ( it->m_name == name ) && !it->m_templateName.empty() )
        {
            templateName = it->m_templateName;
            return true;
        }
    }

    return false;
}

bool ProgramManager::isLoaded( const String& name ) const
{
    Vector< struct LoadedProgram >::const_iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        if( ( it->m_name == name ) || ( it->m_templateName == name ) )
        {
            return true;
        }
    }

    return false;
}

Vector< ExecutionContext::RuntimeError > ProgramManager::runtimeErrors( const String& name )
{
    Vector< ExecutionContext::RuntimeError > runtimeErrors;

    Vector< struct LoadedProgram >::iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        if( it->m_name == name )
        {
            Linda2* linda2( it->m_program );
            if( linda2 )
            {
                Vector< Actors::Linda2Actor* >::const_iterator it( linda2->m_actors.begin() );
                for( ; it != linda2->m_actors.end(); ++it )
                {
                    Actors::Linda2Actor* actor( *it );
                    Vector< ExecutionContext::RuntimeError > thisActorRuntimeErrors( actor->runtimeErrors() );
                    //std::copy( thisActorRuntimeErrors.begin(), thisActorRuntimeErrors.end(), std::back_inserter( runtimeErrors ) );
                    Vector< ExecutionContext::RuntimeError >::const_iterator it2( thisActorRuntimeErrors.begin() );
                    for( ; it2 != thisActorRuntimeErrors.end(); ++it2 )
                    {
                        runtimeErrors.push_back( *it2 );
                    }
                }
            }
        }
    }

    return runtimeErrors;
}

int ProgramManager::readLine( AssetLoader* assetLoader, String& line, int& offset )
{
    line.clear();

    char buffer[bufferSize];

    bool error( false );
    char c( '\0' );
    while( !error && ( offset < assetLoader->size() ) && c != '\n' )
    {
        int numToRead = ( ( offset + bufferSize ) > assetLoader->size() ) ? ( assetLoader->size() - offset ) : bufferSize;
#ifdef LOG_LOADERS
        {
        LiteStream stream;
        stream << "Want to read " << numToRead << " bytes from asset";
        LOG_DEBUG( stream.str() );
        }
#endif
        int numRead( assetLoader->read( buffer, offset, numToRead ) );
#ifdef LOG_LOADERS
        {
        LiteStream stream;
        stream << "Read " << numToRead << " bytes";
        LOG_DEBUG( stream.str() );
        }
#endif
        if( numRead <= 0 )
        {
            LOG_DEBUG( "ProgramManager: Error reading line from asset - zero read" );
            error = true;
            break;
        }

        int lineIdx( 0 );
        for( ; ( lineIdx < numRead ) && ( ( c = buffer[lineIdx] ) != '\n' ); ++lineIdx, ++offset )
        {
            line += c;
        }

        if( c == '\n' )
        {
            ++offset; // Skip newline.
        }
    }

    if( !error && !( line.empty() && offset == assetLoader->size() ) )
    {
#ifdef LOG_LOADERS
        {
        LiteStream stream;
        stream << "Read line of length " << line.size() << ": " << line;
        LOG_DEBUG( stream.str() );
        }
#endif
        return line.size();
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "No line read" );
#endif
    return -1;
}


} // namespace Carlo

} // namespace Agape
