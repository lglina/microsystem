#include "StreamLogger.h"
#include "String.h"
//#include <iostream>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace Agape
{

namespace Loggers
{

void Stream::log( const String& message, LogLevel logLevel )
{
    // FIXME: Implement log levels via base class, which would call a
    // a differently-named derived class function?
    //std::cerr << message.c_str() << std::endl;
#if defined(__EMSCRIPTEN__) && defined(TELA_LOG_SOCKET)
    EM_ASM( {
        fetch("http://127.0.0.1:31337", {
            method: "POST",
            body: UTF8ToString($0)
        }).catch(() => {});
    }, message.c_str() );
#elif defined(__EMSCRIPTEN__) && defined(TELA_LOG_LOCAL_STORAGE)
    EM_ASM( {
        let logs = localStorage.getItem("debug_log") || "";
        logs += UTF8ToString($0) + "\n";
        localStorage.setItem("debug_log", logs);
    }, message.c_str() );
#else
    printf(message.c_str());
    printf("\n");
#endif
}

} // namespace Loggers

} // namespace Agape
