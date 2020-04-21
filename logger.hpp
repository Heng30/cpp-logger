#pragma once

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

class logger {
  public:
    const static int LOGGER_LV_TRACE = 1;
    const static int LOGGER_LV_DEBUG = 2;
    const static int LOGGER_LV_INFO = 4;
    const static int LOGGER_LV_WARN = 8;
    const static int LOGGER_LV_ERROR = 16;
    const static int LOGGER_LV_FATAL = 32;
    const static int LOGGER_LV_ALL = 63;

  private:
    static logger m_lg;
    int m_size;       // 重写大小
    int m_level;            // 日志级别
    std::mutex m_lock;      // 锁
    std::string m_filepath; // 输出文件路径

  public:
    logger(const logger &) = delete;
    logger &operator=(const logger &) = delete;
    logger(const logger &&) = delete;
    logger &operator=(const logger &&) = delete;

    template <typename... Args>
    static void writelog(int level, std::string file, std::string func,
                         int line, Args... args) noexcept;

    static void set_filepath(std::string filepath) noexcept;
    static void set_size(int size) noexcept;
    static void set_level(int level) noexcept;
    static void dump() noexcept;
    static void test() noexcept;

  private:
    logger();
    std::string _levelc(int level) noexcept;

    template <typename T, typename U> void _write(U &out, const T &t) noexcept;

    template <typename T, typename U, typename... Args>
    void _write(U &out, const T &t, Args... args) noexcept;
};

template <typename T, typename U>
void logger::_write(U &out, const T &t) noexcept {
    out << t << std::endl;
}

template <typename T, typename U, typename... Args>
void logger::_write(U &out, const T &t, Args... args) noexcept {
    out << t;
    _write(out, args...); //利用模板推导机制，每次取出第一个，缩短参数包的大小
}

template <typename... Args>
void logger::writelog(int level, std::string file, std::string func, int line,
                      Args... args) noexcept {
    if (!(level & m_lg.m_level))
        return;

    time_t nt = time((time_t *)nullptr);
    struct tm ltm;
    char buf[64] = {0};
    struct stat st;
    std::ofstream outfile;
    std::string lc = m_lg._levelc(level);
    std::ios_base::openmode mode = std::ios::out | std::ios::app;

    localtime_r(&nt, &ltm);
    strftime(buf, sizeof(buf), "%b %d %T", &ltm);
    std::string str = "[" + std::string(lc) + "] " + "[" + std::string(buf) +
                      "] " + "[" + file + "] " + "[" + func + ":";

    std::lock_guard<std::mutex> lk(m_lg.m_lock);

    if (m_lg.m_filepath.length() <= 0) {
        std::cout << str << line << "]: ";
        m_lg._write(std::cout, args...);
    } else {
        if (lstat(m_lg.m_filepath.c_str(), &st) < 0) {
            std::cout << str << line << "]: ";
            std::cout << "lstat " << m_lg.m_filepath
                      << " error, errno: " << errno << " - " << strerror(errno)
                      << std::endl;
        } else {
            if (st.st_size > m_lg.m_size)
                mode = std::ios::out | std::ios::trunc;
        }

        outfile.open(m_lg.m_filepath, mode);
        if (!outfile.is_open()) {
            std::cout << str << line << "]: ";
            std::cout << "open " << m_lg.m_filepath
                      << " error, errno: " << errno << " - " << strerror(errno)
                      << std::endl;
        } else {
            outfile << str << line << "]: ";
            m_lg._write(outfile, args...);
            outfile.close();
        }
    }
}

#define _TRACE_LOG(...)                                                        \
    logger::writelog(int(logger::LOGGER_LV_TRACE), std::string(__FILE__),      \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)

#define _DEBUG_LOG(...)                                                        \
    logger::writelog(int(logger::LOGGER_LV_DEBUG), std::string(__FILE__),      \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)

#define _INFO_LOG(...)                                                         \
    logger::writelog(int(logger::LOGGER_LV_INFO), std::string(__FILE__),       \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)

#define _WARN_LOG(...)                                                         \
    logger::writelog(int(logger::LOGGER_LV_WARN), std::string(__FILE__),       \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)

#define _ERROR_LOG(...)                                                        \
    logger::writelog(int(logger::LOGGER_LV_ERROR), std::string(__FILE__),      \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)

#define _FATAL_LOG(...)                                                        \
    logger::writelog(int(logger::LOGGER_LV_FATAL), std::string(__FILE__),      \
                     std::string(__FUNCTION__), int(__LINE__), ##__VA_ARGS__)
