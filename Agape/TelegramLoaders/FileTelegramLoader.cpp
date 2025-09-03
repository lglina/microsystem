#include "World/Telegram.h"
#include "Collections.h"
#include "FileTelegramLoader.h"
#include "FileWriter.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

using namespace Agape::World;

namespace Agape
{

namespace TelegramLoaders
{

File::File( const String& recipientSnowflake,
            const String& filename,
            const String& worldID ) :
  TelegramLoader( recipientSnowflake ),
  m_filename( filename ),
  m_worldID( worldID )
{
}

bool File::load( Vector< Telegram >& telegrams )
{
    FileWriter fileWriter( m_filename, FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( fileWriter, telegramsValue ) )
        {
            ConstListIterator it( telegramsValue.listBegin() );
            for( ; it != telegramsValue.listEnd(); ++it )
            {
                Telegram thisTelegram( Telegram::fromValue( **it ) );
                if( thisTelegram.m_recipientSnowflake == m_recipientSnowflake )
                {
                    telegrams.push_back( thisTelegram );
                }
            }

            return true;
        }
    }

    return false;
}

bool File::loadSent( Vector< Telegram >& telegrams )
{
    FileWriter fileWriter( m_filename, FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( fileWriter, telegramsValue ) )
        {
            ConstListIterator it( telegramsValue.listBegin() );
            for( ; it != telegramsValue.listEnd(); ++it )
            {
                Telegram thisTelegram( Telegram::fromValue( **it ) );
                if( thisTelegram.m_senderSnowflake == m_recipientSnowflake )
                {
                    telegrams.push_back( thisTelegram );
                }
            }

            return true;
        }
    }

    return false;
}

bool File::send( const Telegram& telegram )
{
    FileWriter fileWriter( m_filename, FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( fileWriter, telegramsValue ) )
        {
            Value* telegramValue = new Value;
            telegram.toValue( *telegramValue );
            telegramsValue.push_back( telegramValue );

            return true;
        }
    }

    return false;
}

bool File::markRead( const Telegram& telegram )
{
    FileWriter readFileWriter( m_filename, FileWriter::modeRead );

    if( readFileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( readFileWriter, telegramsValue ) )
        {
            readFileWriter.close();

            ListIterator it( telegramsValue.listBegin() );
            for( ; it != telegramsValue.listEnd(); ++it )
            {
                Telegram thisTelegram( Telegram::fromValue( **it ) );
                if( thisTelegram == telegram )
                {
                    thisTelegram.m_unread = false;
                    thisTelegram.toValue( **it );
                    break;
                }
            }

            if( it != telegramsValue.listEnd() )
            {
                FileWriter writeFileWriter( m_filename, FileWriter::modeWrite );

                if( writeFileWriter.isOpen() )
                {
                    telegramsValue.toReadableWritable( writeFileWriter );
                    return true;
                }
            }
        }
    }

    return false;
}

bool File::erase( const Telegram& telegram )
{
    FileWriter readFileWriter( m_filename, FileWriter::modeRead );

    if( readFileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( readFileWriter, telegramsValue ) )
        {
            readFileWriter.close();

            ListIterator it( telegramsValue.listBegin() );
            for( ; it != telegramsValue.listEnd(); ++it )
            {
                Telegram thisTelegram( Telegram::fromValue( **it ) );
                if( thisTelegram == telegram )
                {
                    telegramsValue.erase( it );
                    break;
                }
            }

            if( it != telegramsValue.listEnd() )
            {
                FileWriter writeFileWriter( m_filename, FileWriter::modeWrite );

                if( writeFileWriter.isOpen() )
                {
                    telegramsValue.toReadableWritable( writeFileWriter );
                    return true;
                }
            }
        }
    }

    return false;
}

bool File::unread( Map< String, int >& numUnread, bool allDevices )
{
    numUnread.clear();
    numUnread[m_worldID] = 0;

    FileWriter fileWriter( m_filename, FileWriter::modeRead );

    if( fileWriter.isOpen() )
    {
        Value telegramsValue;
        if( Value::fromReadableWritable( fileWriter, telegramsValue ) )
        {
            ConstListIterator it( telegramsValue.listBegin() );
            for( ; it != telegramsValue.listEnd(); ++it )
            {
                Telegram thisTelegram( Telegram::fromValue( **it ) );
                if( ( thisTelegram.m_recipientSnowflake == m_recipientSnowflake ) &&
                    thisTelegram.m_unread )
                {
                    ++numUnread[m_worldID];
                }
            }
        }
        return true;
    }

    return false;
}

} // namespace TelegramLoaders

} // namespace Agape
