#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <mutex>
#include <string_view>
#include <utility>

namespace kine::log
{
enum class Level
{
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Todo,
    Unreachable
};

#ifndef NDEBUG
inline Level min_level = Level::Trace;
#else
inline Level min_level = Level::Info;
#endif

#ifndef LOG_THREAD_SAFE
#    define LOG_THREAD_SAFE 1
#endif

#if LOG_THREAD_SAFE
inline std::mutex log_mutex;
#endif

inline constexpr std::string_view level_name(Level lvl) noexcept
{
    switch (lvl)
    {
    case Level::Trace:
        return "TRACE";
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO ";
    case Level::Warn:
        return "WARN ";
    case Level::Error:
        return "ERROR";
    case Level::Critical:
        return "CRIT ";
    case Level::Todo:
        return "TODO ";
    case Level::Unreachable:
        return "UNREACHABLE";
    default:
        return "UNKWN";
    }
}

inline bool use_color = true;

inline const std::string_view level_color(Level lvl) noexcept
{
    if (!use_color) return "";

    switch (lvl)
    {
    case Level::Trace:
        return "\033[90m";
    case Level::Debug:
    case Level::Todo:
        return "\033[36m";
    case Level::Info:
        return "\033[32m";
    case Level::Warn:
        return "\033[33m";
    case Level::Error:
        return "\033[31m";
    case Level::Critical:
    case Level::Unreachable:
        return "\033[41m\033[97m";
    default:
        return "";
    }
}

inline std::string time_string()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return buf;
}

using Handler = void (*)(Level, std::string_view);

inline void default_handler(Level lvl, std::string_view message)
{
    std::cout << '[' << time_string() << "] " << level_color(lvl) << level_name(lvl) << ' ' << message;

    if (use_color) std::cout << "\033[0m";

    if (!message.ends_with('\n')) std::cout << '\n';

    std::cout.flush();
}

inline Handler handler = default_handler;

inline void set_handler(Handler h) noexcept { handler = h ? h : default_handler; }

inline Handler get_handler() noexcept { return handler; }

inline void set_level(Level lvl) noexcept { min_level = lvl; }

template <typename... Args>
inline void write(Level lvl, std::format_string<Args...> fmt, Args&&... args)
{
    if (lvl < min_level || !handler) return;

#if LOG_THREAD_SAFE
    std::scoped_lock lock(log_mutex);
#endif

    handler(lvl, std::format(fmt, std::forward<Args>(args)...));
}

}  // namespace kine::log

//// MACROS /////

#define UNUSED(value) (void) (value)

#define LOG_TRACE(...) kine::log::write(kine::log::Level::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) kine::log::write(kine::log::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...) kine::log::write(kine::log::Level::Info, __VA_ARGS__)
#define LOG_WARN(...) kine::log::write(kine::log::Level::Warn, __VA_ARGS__)
#define LOG_ERROR(...) kine::log::write(kine::log::Level::Error, __VA_ARGS__)
#define LOG_CRITICAL(...) kine::log::write(kine::log::Level::Critical, __VA_ARGS__)

#define LOG_THROW(...)          \
    do                          \
    {                           \
        LOG_ERROR(__VA_ARGS__); \
        std::abort();           \
    } while (0)

#define LOG_TODO(...)                                          \
    do                                                         \
    {                                                          \
        kine::log::write(kine::log::Level::Todo, __VA_ARGS__); \
        std::abort();                                          \
    } while (0)

#define LOG_UNREACHABLE(...)                                          \
    do                                                                \
    {                                                                 \
        kine::log::write(kine::log::Level::Unreachable, __VA_ARGS__); \
        std::abort();                                                 \
    } while (0)
