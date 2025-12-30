#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef LOG_THREAD_SAFE
#    define LOG_THREAD_SAFE 1
#endif

#if LOG_THREAD_SAFE
#    if defined(_WIN32)
#        include <windows.h>
static CRITICAL_SECTION log_mutex;
static int log_mutex_initialized = 0;
#        define LOG_LOCK()                                 \
            do                                             \
            {                                              \
                if (!log_mutex_initialized)                \
                {                                          \
                    InitializeCriticalSection(&log_mutex); \
                    log_mutex_initialized = 1;             \
                }                                          \
                EnterCriticalSection(&log_mutex);          \
            } while (0)
#        define LOG_UNLOCK() LeaveCriticalSection(&log_mutex)
#    else
#        include <pthread.h>
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
#        define LOG_LOCK() pthread_mutex_lock(&log_mutex)
#        define LOG_UNLOCK() pthread_mutex_unlock(&log_mutex)
#    endif
#else
#    define LOG_LOCK()
#    define LOG_UNLOCK()
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define MAYBE_UNUSED __attribute__((unused))
#    ifdef __MINGW_PRINTF_FORMAT
#        define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) \
            __attribute__((format(__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#    else
#        define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) \
            __attribute__((format(printf, STRING_INDEX, FIRST_TO_CHECK)))
#    endif  // __MINGW_PRINTF_FORMAT
#else
#    define MAYBE_UNUSED
//   TODO: implement PRINTF_FORMAT for MSVC.
#    define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

typedef enum LogLevel
{
    LOGL_TRACE = 0,
    LOGL_DEBUG,
    LOGL_INFO,
    LOGL_WARN,
    LOGL_ERROR,
    LOGL_CRITICAL,
    LOGL_TODO,
    LOGL_UNREACHABLE
} LogLevel;

#ifndef NDEBUG
static LogLevel log_min_level = LOGL_TRACE;
#else
static LogLevel log_min_level = LOG_INFO;
#endif

static MAYBE_UNUSED void log_write(LogLevel level, const char* fmt, ...) PRINTF_FORMAT(2, 3);

static void log_default_handler(LogLevel level, const char* fmt, va_list args);

typedef void(LogHandler)(LogLevel level, const char* fmt, va_list args);

static LogHandler* log_handler = log_default_handler;

static MAYBE_UNUSED void log_write(LogLevel level, const char* fmt, ...)
{
    if (level < log_min_level || !log_handler) return;

    va_list args;
    va_start(args, fmt);

    LOG_LOCK();
    log_handler(level, fmt, args);
    LOG_UNLOCK();

    va_end(args);
}

static MAYBE_UNUSED void log_set_level(LogLevel level) { log_min_level = level; }

static MAYBE_UNUSED void log_set_handler(LogHandler handler) { log_handler = handler ? handler : log_default_handler; }
static MAYBE_UNUSED LogHandler* log_get_handler(void) { return log_handler; }

///// DEFAULT HANDLER /////

static const char* log_level_name(LogLevel level)
{
    switch (level)
    {
    case LOGL_TRACE:
        return "TRACE";
    case LOGL_DEBUG:
        return "DEBUG";
    case LOGL_INFO:
        return "INFO ";
    case LOGL_WARN:
        return "WARN ";
    case LOGL_ERROR:
        return "ERROR";
    case LOGL_CRITICAL:
        return "CRIT ";
    case LOGL_TODO:
        return "TODO ";
    case LOGL_UNREACHABLE:
        return "UNREACHABLE ";
    default:
        return "UNKWN";
    }
}

static int log_use_color = 1;

static const char* log_level_color(LogLevel level)
{
    if (!log_use_color) return "";

    switch (level)
    {
    case LOGL_TRACE:
        return "\033[90m";
    case LOGL_TODO:
    case LOGL_DEBUG:
        return "\033[36m";
    case LOGL_INFO:
        return "\033[32m";
    case LOGL_WARN:
        return "\033[33m";
    case LOGL_ERROR:
        return "\033[31m";
    case LOGL_UNREACHABLE:
    case LOGL_CRITICAL:
        return "\033[41m\033[97m";
    default:
        return "";
    }
}

static void log_get_time(char* buf, size_t size)
{
    time_t t = time(NULL);
    struct tm tm;

#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    strftime(buf, size, "%H:%M:%S", &tm);
}

static void log_default_handler(LogLevel level, const char* fmt, va_list args)
{
    char time_buf[16];
    log_get_time(time_buf, sizeof(time_buf));

    // Prefix
    fprintf(stdout, "[%s] %s%s ", time_buf, log_level_color(level), log_level_name(level));

    // Message
    vfprintf(stdout, fmt, args);

    // Reset color
    if (log_use_color) fputs("\033[0m", stdout);

    fputc('\n', stdout);
}

//// MACROS /////

#define UNUSED(value) (void) (value)

#define LOG_TRACE(...) log_write(LOGL_TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) log_write(LOGL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) log_write(LOGL_INFO, __VA_ARGS__)
#define LOG_WARN(...) log_write(LOGL_WARN, __VA_ARGS__)
#define LOG_ERROR(...) log_write(LOGL_ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) log_write(LOGL_CRITICAL, __VA_ARGS__)

#define LOG_THROW(...)                      \
    do                                      \
    {                                       \
        log_write(LOGL_ERROR, __VA_ARGS__); \
        abort();                            \
    } while (0)

#define LOG_TODO(...)                      \
    do                                     \
    {                                      \
        log_write(LOGL_TODO, __VA_ARGS__); \
        abort();                           \
    } while (0)

#define LOG_UNREACHABLE(...)                      \
    do                                            \
    {                                             \
        log_write(LOGL_UNREACHABLE, __VA_ARGS__); \
        abort();                                  \
    } while (0)
