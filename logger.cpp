#include "logger.hpp"

using namespace std;

logger logger::m_lg;

logger::logger() {
    m_lg.m_filepath = string{};
    m_lg.m_level = LOGGER_LV_ALL;
    m_lg.m_size = 1024 * 1024;
}

void logger::set_filepath(string filepath) noexcept {
    std::lock_guard<std::mutex> lk(m_lg.m_lock);
    m_lg.m_filepath = filepath;
}

void logger::set_size(int size) noexcept {
    std::lock_guard<std::mutex> lk(m_lg.m_lock);
    m_lg.m_size = size;
}

void logger::set_level(int level) noexcept {
    std::lock_guard<std::mutex> lk(m_lg.m_lock);
    m_lg.m_level = level;
}

void logger::dump() noexcept {
    _TRACE_LOG("filepath: ", m_lg.m_filepath);
    _TRACE_LOG("level: ", m_lg.m_level);
    _TRACE_LOG("size: ", m_lg.m_size);
}

string logger::_levelc(int level) noexcept {
    switch (level) {
    case LOGGER_LV_TRACE:
        return "T";
    case LOGGER_LV_DEBUG:
        return "D";
    case LOGGER_LV_INFO:
        return "I";
    case LOGGER_LV_WARN:
        return "W";
    case LOGGER_LV_ERROR:
        return "E";
    case LOGGER_LV_FATAL:
        return "F";
    }
    return "U";
}

#ifdef _TESTING

void logger::test() noexcept {
    const char *filepath = "./logger.log";
    /* logger::set_filepath(filepath); */
    logger::set_level(LOGGER_LV_ALL);
    logger::set_size(1024);
    logger::dump();

    auto log = []() {
        for (int i = 0; i < 1024; i++) {
            _TRACE_LOG("hello ", i, " world");
            _DEBUG_LOG("hello ", i, " world");
            _INFO_LOG("hello ", i, " world");
            _WARN_LOG("hello ", i, " world");
            _ERROR_LOG("hello ", i, " world");
            _FATAL_LOG("hello ", i, " world");
        }
    };

    thread td1(log), td2(log), td3(log);
    td1.join();
    td2.join();
    td3.join();
    unlink(filepath);

    std::cout << __FILE__ << " " << __FUNCTION__ << " OK" << std::endl;
}

int main(void) { logger::test(); }
#endif
