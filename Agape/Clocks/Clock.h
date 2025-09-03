#ifndef AGAPE_CLOCK_H
#define AGAPE_CLOCK_H

#include "String.h"

namespace Agape
{

class Clock
{
public:
    virtual ~Clock() {}
    
    virtual String dateTime() = 0;
    virtual void fillDateTime( char* dateTime ) = 0;

    virtual unsigned long long epochS() { return epochMS() / 1000; };
    virtual unsigned long long epochMS() = 0;

    static void timestampToParts( long long timestamp,
                                  int& day,
                                  int& month,
                                  int& year,
                                  int& hour,
                                  int& minute,
                                  int& second,
                                  int& dayOfWeek );
    static void timestampToISO( long long timestamp,
                                String& isoDateTime );
    static void isoToParts( const String& isoDateTime,
                            int& day,
                            int& month,
                            int& year,
                            int& hour,
                            int& minute,
                            int& second );

    static void utcToVRT( int& day,
                          int& month,
                          int& year,
                          int& hour,
                          int& minute,
                          int& second,
                          int& dayOfWeek );
    static void vrtToUTC( int& day,
                          int& month,
                          int& year,
                          int& hour,
                          int& minute,
                          int& second,
                          int& dayOfWeek );

    static int daysInYear( int year );
    static int daysInMonth( int month, int year );
    static bool isLeap( int year );

    static const char* dayOfWeekName( int dayOfWeek );
    static const char* monthName( int month );

    static String dateTimeToString( int& day,
                                    int& month,
                                    int& year,
                                    int& hour,
                                    int& minute,
                                    int& second,
                                    int& dayOfWeek );

    static String intervalToString( int interval );
    static String formatISO( const String& dateTime );
};

} // namespace Agape

#endif // AGAPE_CLOCK_H
