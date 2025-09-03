#include "Utils/LiteStream.h"
#include "Clock.h"
#include "String.h"

#include <cstdlib>

namespace
{
    const char* dayNames[7] =
    {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
    };

    const char* monthNames[12] =
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };
} // Anonymous namespace

namespace Agape
{

void Clock::timestampToParts( long long timestamp,
                              int& day,
                              int& month,
                              int& year,
                              int& hour,
                              int& minute,
                              int& second,
                              int& dayOfWeek )
{
    // Calculate Y-M-D.
    int totalDays( timestamp / ( 24 * 60 * 60 ) );
    dayOfWeek = ( ( totalDays - 3 ) % 7 ) + 1;
    
    year = 1970;
    int ydays( daysInYear( year ) );
    while( totalDays >= ydays )
    {
        totalDays -= ydays;
        ++year;
        ydays = daysInYear( year );
    }

    month = 0;
    int mdays( daysInMonth( month + 1, year ) );
    while( totalDays >= mdays )
    {
        totalDays -= mdays;
        ++month;
        mdays = daysInMonth( month + 1, year );
    }
    ++month; // Make month one-based.

    day = totalDays + 1; // Make day one-based.

    // Calculate H-M-S
    int daySeconds( timestamp % ( 24 * 60 * 60 ) );
    hour = daySeconds / ( 60 * 60 );
    minute = ( daySeconds % ( 60 * 60 ) ) / 60;
    second = ( daySeconds % ( 60 * 60 ) ) % 60;
}

void Clock::timestampToISO( long long timestamp,
                            String& isoDateTime )
{
    int day = 1, month = 1, year = 0;
    int hour = 0, minute = 0, second = 0;
    int dayOfWeek = 1;

    timestampToParts( timestamp,
                      day,
                      month,
                      year,
                      hour,
                      minute,
                      second,
                      dayOfWeek );

    LiteStream stream;
    stream << year;
    if( day < 10 ) stream << "0"; stream << day;
    if( month < 10 ) stream << "0"; stream << month;
    if( hour < 10 ) stream << "0"; stream << hour;
    if( minute < 10 ) stream << "0"; stream << minute;
    if( second < 10 ) stream << "0"; stream << second;
    isoDateTime = stream.str();
}

void Clock::isoToParts( const String& isoDateTime,
                        int& day,
                        int& month,
                        int& year,
                        int& hour,
                        int& minute,
                        int& second )
{
    if( isoDateTime.length() == 14 )
    {
        day = ::atoi( isoDateTime.substr( 6, 2 ).c_str() );
        month = ::atoi( isoDateTime.substr( 4, 2 ).c_str() );
        year = ::atoi( isoDateTime.substr( 0, 4 ).c_str() );
        hour = ::atoi( isoDateTime.substr( 8, 2 ).c_str() );
        minute = ::atoi( isoDateTime.substr( 10, 2 ).c_str() );
        second = ::atoi( isoDateTime.substr( 12, 2 ).c_str() );
    }
}

void Clock::utcToVRT( int& day,
                      int& month,
                      int& year,
                      int& hour,
                      int& minute,
                      int& second,
                      int& dayOfWeek )
{
    hour -= 2;
    if( hour < 0 )
    {
        hour += 24;
        --day;
        --dayOfWeek;
        if( dayOfWeek == 0 ) dayOfWeek = 7;
        if( day < 1 )
        {
            --month;
            if( month < 1 )
            {
                month = 12;
                --year;
            }
            day = daysInMonth( month, year );
        }
    }
}

void Clock::vrtToUTC( int& day,
                      int& month,
                      int& year,
                      int& hour,
                      int& minute,
                      int& second,
                      int& dayOfWeek )
{
    hour += 2;
    if( hour > 23 )
    {
        hour -= 24;
        ++day;
        ++dayOfWeek;
        if( dayOfWeek == 8 ) dayOfWeek = 1;
        if( day > daysInMonth( month, year ) )
        {
            ++month;
            if( month > 12 )
            {
                month = 1;
                ++year;
            }
        }
    }
}

int Clock::daysInYear( int year )
{
    if( isLeap( year ) )
    {
        return 366;
    }

    return 365;
}

int Clock::daysInMonth( int month, int year )
{
    if( month == 2 )
    {
        if( isLeap( year ) )
        {
            return 29;
        }
        return 28;
    }

    if( month <= 7 )
    {
        if( ( month % 2 ) )
        {
            return 31;
        }
        return 30;
    }

    if( ( month % 2 ) )
    {
        return 30;
    }
    return 31;
}

bool Clock::isLeap( int year )
{
    if( !( year % 400 ) ||
        ( !( year % 4 ) && ( year % 100 ) ) )
    {
        return true;
    }

    return false;
}

const char* Clock::dayOfWeekName( int dayOfWeek )
{
    return dayNames[dayOfWeek - 1];
}

const char* Clock::monthName( int month )
{
    return monthNames[month - 1];
}

String Clock::dateTimeToString( int& day,
                                int& month,
                                int& year,
                                int& hour,
                                int& minute,
                                int& second,
                                int& dayOfWeek )
{
    LiteStream dateTime;

    dateTime << dayOfWeekName( dayOfWeek ) << " "
             << day << " "
             << monthName( month ) << " "
             << year << " ";

    if( hour < 10 ) dateTime << "0";
    dateTime << hour << ":";

    if( minute < 10 ) dateTime << "0";
    dateTime << minute << ":";

    if( second < 10 ) dateTime << "0";
    dateTime << second << " ";

    dateTime << "VRT";

    return dateTime.str();
}

String Clock::intervalToString( int interval )
{
    LiteStream stream;
    
    if( interval < 60 )
    {
        stream << interval << "s";
    }
    else if( interval <= ( 60 * 60 ) )
    {
        stream << ( interval / 60 ) << "m";
    }
    else if( interval <= ( 60 * 60 * 24 ) )
    {
        stream << ( interval / 60 / 60 ) << "h";
    }
    else if( interval <= ( 60 * 60 * 24 * 7 ) )
    {
        stream << ( interval / 60 / 60 / 24 ) << "d";
    }
    else if( interval <= ( 60 * 60 * 24 * 7 * 52 ) )
    {
        stream << ( interval / 60 / 60 / 24 / 7 ) << "w";
    }
    else
    {
        stream << ">1y";
    }

    return stream.str();
}

String Clock::formatISO( const String& isoDateTime )
{
    int day = 1, month = 1, year = 0;
    int hour = 0, minute = 0, second = 0;
    int dayOfWeek = 1;

    isoToParts( isoDateTime,
                day,
                month,
                year,
                hour,
                minute,
                second );

    utcToVRT( day,
              month,
              year,
              hour,
              minute,
              second,
              dayOfWeek );

    LiteStream stream;
    if( day < 10 ) stream << "0"; stream << day << "/";
    if( month < 10 ) stream << "0"; stream << month << "/";
    stream << year << " ";
    if( hour < 10 ) stream << "0"; stream << hour << ":";
    if( minute < 10 ) stream << "0"; stream << minute << ":";
    if( second < 10 ) stream << "0"; stream << second;
    stream << " VRT";
    return stream.str();
}

} // namespace Agape
