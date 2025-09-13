#include <Logger.h>

Logger& Logger::Locate() {
    static Logger instance;
    return instance;
}
