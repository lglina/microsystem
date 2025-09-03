#include "MidiFile.h"
#include "Options.h"

#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <unistd.h>

using namespace std;
using namespace smf;

namespace
{
    const double frobsPerSec( 3906.25 );
} // Anonymous namespace

int main( int argc, char** argv )
{
    Options options;
    options.process( argc, argv );

    MidiFile midifile;
    if( options.getArgCount() > 0 )
    {
        midifile.read( options.getArg( 1 ) );
    }
    else
    {
        midifile.read( cin );
    }

    int outputFd( -1 );
    if( options.getArgCount() == 2 )
    {
        outputFd = ::open( options.getArg( 2 ).c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    }

    midifile.joinTracks();
    midifile.doTimeAnalysis();
    
    MidiEvent* mev;
    int numBytes( 0 );
    for( int event = 0; event < midifile[0].size(); ++event )
    {
        mev = &midifile[0][event];
        if( ( *mev )[0] == 0xFF ) continue; // Filter out meta-events.
        if( mev->size() > 64 ) continue; // Filter out too-long SYSEX.
        
        int frob( (int)round( mev->seconds * frobsPerSec ) );

        //cout << "0x" << hex << ( ( frob >> 24 ) & 0xFF ) << ", ";
        int firstByte( mev->size() << 4 );
        firstByte += ( ( frob >> 16 ) & 0x0F );
        cout << "0x" << hex << firstByte << ", ";
        cout << "0x" << hex << ( ( frob >> 8 ) & 0xFF ) << ", ";
        cout << "0x" << hex << ( frob & 0xFF ) << ", ";
        //cout << "0x" << hex << mev->size() << ", ";
        for( int i = 0; i < mev->size(); ++i )
        {
            cout << "0x" << hex << (int)( *mev )[i] << ", ";
        }
        cout << endl;

        if( outputFd >= 0 )
        {
            char c;
            //c = ( frob >> 24 ) & 0xFF; ::write( outputFd, &c, 1 );
            firstByte = mev->size() << 4;
            firstByte += ( ( frob >> 16 ) & 0x0F );
            ::write( outputFd, &firstByte, 1 );
            c = ( frob >> 8 ) & 0xFF; ::write( outputFd, &c, 1 );
            c = frob & 0xFF; ::write( outputFd, &c, 1 );
            //c = mev->size(); ::write( outputFd, &c, 1 );
            for( int i = 0; i < mev->size(); ++i )
            {
                c = ( *mev )[i];
                ::write( outputFd, &c, 1 );
            }
        }
        
        numBytes += 3 + mev->size();
    }
    cout << std::dec << numBytes << endl;

    ::close( outputFd );    

    return 0;
}
