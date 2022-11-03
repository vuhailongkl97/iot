#include <logger.h>
#include <cstring>
#include <iostream>
#include "../src/logger.cpp"
template<typename... T>
void log(LOGLV lv, const char* fmt, T&&... args)
{
    char buf[150];
    Logger& lg = spdLogger().getInstance();
    snprintf(buf, sizeof(buf) - 1, fmt, std::forward<T>(args)...);

    switch (lv) {
        case LOGLV::INFO: lg.info(buf); break;
        case LOGLV::ERROR: lg.error(buf); break;
        case LOGLV::WARNING: lg.warn(buf); break;
        case LOGLV::DEBUG: lg.debug(buf); break;
        default: lg.info(buf);
    }
}

int main()
{

    log(LOGLV::INFO, "%s - %d", "hello", 1234);
    log(LOGLV::WARNING, "%s - %d", "hello", 1234);
    log(LOGLV::ERROR, "%s - %d", "hello", 1234);

    return 0;
}
