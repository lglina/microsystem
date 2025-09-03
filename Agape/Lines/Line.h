#ifndef AGAPE_LINE_H
#define AGAPE_LINE_H

#include "LineDrivers/LineDriver.h"
#include "Collections.h"
#include "ReadableWritable.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

class Line : public ReadableWritable, public Runnable
{
public:
    enum ConfigOptionTypes
    {
        unknown,
        text,
        encodedText,
        select
    };

    struct ConfigOption
    {
        ConfigOption() :
          m_type( unknown )
        {
        }

        ConfigOption( const String& name,
                      enum ConfigOptionTypes type,
                      const String& alternatives ) :
          m_name( name ),
          m_type( type ),
          m_alternatives( alternatives )
        {
        }

        String m_name;
        enum ConfigOptionTypes m_type;
        String m_alternatives;
    };

    struct LineStatus
    {
        LineStatus() :
          m_ready( false ),
          m_ringing( false ),
          m_carrier( false ),
          m_secure( false )
        {
        }

        bool operator!=( const LineStatus& other )
        {
            return( other.m_ready != m_ready ||
                    other.m_ringing != m_ringing ||
                    other.m_carrier != m_carrier ||
                    other.m_secure != m_secure ||
                    other.m_encryptorStatus != m_encryptorStatus );
        }

        bool m_ready;
        bool m_ringing;
        bool m_carrier;
        bool m_secure;
        String m_encryptorStatus;
    };

    Line( LineDriver& lineDriver );
    virtual ~Line() {};

    virtual void open();
    virtual void run();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    virtual Vector< Line::ConfigOption > getConfigOptions() = 0;
    virtual void setConfigOption( const String& name, const String& value ) = 0;

    virtual void connect() = 0;
    virtual void registerNumber( const String& number ) = 0;

    virtual void dial( const String& number ) = 0;
    virtual void answer() = 0;
    virtual void hangup() = 0;

    virtual struct Line::LineStatus getLineStatus() = 0;

    void setRequiresAuthentication( bool requiresAuthentication );
    bool requiresAuthentication() const;

protected:
    LineDriver& m_lineDriver;

    struct LineStatus m_lineStatus;
    int m_lineStatusCounter;

    bool m_requiresAuthentication;
};

} // namespace Agape

#endif // AGAPE_LINE_H
