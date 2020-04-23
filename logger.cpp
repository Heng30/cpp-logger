#include "logger.hpp"
#include <climits>

using namespace std;

logger logger::m_lg;

logger::logger() {
    m_lg.m_filepath = string{};
    m_lg.m_level = LOGGER_LV_ALL;
    m_lg.m_size = 1024 * 1024;
}

static bool _is_dir_exist(const char *dir) {
    if (!dir)
        return false;

    struct stat st;
    if (lstat(dir, &st))
        return false;

    return S_ISDIR(st.st_mode);
}

static bool _create_dir(const char *dir, mode_t mode) {
    if (!dir)
        return false;

    char dir_buf[PATH_MAX] = {0};
    char *start = dir_buf;
    char ch = 0;

    if (!strchr(dir, '/'))
        return false;

    snprintf(dir_buf, sizeof(dir_buf), "%s/", dir);

    while ((start = strchr(start, '/'))) {
        ch = *(start + 1);
        *(start + 1) = '\0';
        if (_is_dir_exist(dir_buf))
            goto next;

        if (-1 == mkdir(dir_buf, mode)) {
            printf("mkdir %s error, errno: %d - %s\n", dir_buf, errno,
                   strerror(errno));
            return false;
        }

    next:
        *(start + 1) = ch;
        start++;
    }
    return true;
}

void logger::set_filepath(string filepath) noexcept {
    const char *ptr = nullptr, *start = filepath.c_str();
    char path[PATH_MAX] = {0};
    size_t len = 0;

    std::lock_guard<std::mutex> lk(m_lg.m_lock);
    m_lg.m_filepath = filepath;

    if (ptr = strrchr(const_cast<char *>(start), '/'), ptr) {
        len = (size_t)(ptr - start);
        memcpy(path, start, len);
    }

    if (!_is_dir_exist(path))
        _create_dir(path, 0755);
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
    const char *filepath = "./a/logger.log";
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
