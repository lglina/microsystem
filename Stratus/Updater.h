#ifndef AGAPE_STRATUS_UPDATER
#define AGAPE_STRATUS_UPDATER

#include "Actors/NativeActors/NativeActor.h"
#include "String.h"

#include <cstdio>
#include <filesystem>
#include <map>
#include <string>

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

using namespace std::filesystem;

namespace Stratus
{

class Authenticator;

class Updater : public Actors::Native
{
public:
    Updater( TupleRouter& tupleRouter, Authenticator& authenticator );
    ~Updater();

    virtual bool accept( Tuple& tuple );

private:
    struct UpdateMetadata
    {
        int m_version;
        file_time_type m_lastModified;
    };

    void updateFromFilesystem();
    int versionFromFile( const std::string& filename );

    TupleRouter& m_tupleRouter;
    Authenticator& m_authenticator;

    std::vector< std::string > m_allowedStreams;

    std::map< std::string, UpdateMetadata > m_streamMetadata;

    std::FILE* m_currentFile;
    int m_currentFileSize;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_UPDATER
