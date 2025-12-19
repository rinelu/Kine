#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <source_location>
#include <sstream>
#include <string_view>

#define UNUSED(a) (void) a;
// #define NDEBUG

class Log
{
   public:
    enum class Level : uint8_t
    {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    static inline bool use_color = true;

    template <typename T>
    static constexpr std::string_view type_name()
    {
        return detail::extract(detail::wrapped_type_name<T>());
    }

#ifdef NDEBUG
    static inline Level min_level = Level::Info;
#else
    static inline Level min_level = Level::Trace;
#endif

   private:
    static inline std::mutex write_mutex;

    struct detail
    {
        template <typename T>
        static constexpr std::string_view wrapped_type_name()
        {
#if defined(__clang__)
            return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
            return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
            return __FUNCSIG__;
#else
            return "Unsupported compiler";
#endif
        }

        static constexpr std::string_view extract(std::string_view wrapped)
        {
#if defined(__clang__)
            // "... [T = MyType]"
            auto start = wrapped.find("T = ") + 4;
            auto end = wrapped.rfind(']');
            return wrapped.substr(start, end - start);

#elif defined(__GNUC__)
            // "... [with T = MyType]"
            auto start = wrapped.find("T = ") + 4;
            auto end = wrapped.rfind(']');
            return wrapped.substr(start, end - start);

#elif defined(_MSC_VER)
            // "... wrapped_type_name<MyType>(void)"
            auto start = wrapped.find("wrapped_type_name<") + 18;
            auto end = wrapped.find(">(void)");
            return wrapped.substr(start, end - start);
#endif
        }
    };

    static constexpr std::string_view level_name(Level lvl) noexcept
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
        }
        return "UNKWN";
    }

    static const char* level_color(Level lvl) noexcept
    {
        if (!use_color) return "";
        switch (lvl)
        {
        case Level::Trace:
            return "\033[90m";
        case Level::Debug:
            return "\033[36m";
        case Level::Info:
            return "\033[32m";
        case Level::Warn:
            return "\033[33m";
        case Level::Error:
            return "\033[31m";
        case Level::Critical:
            return "\033[41m\033[97m";
        }
        return "";
    }

    static std::tm local_time(std::time_t t)
    {
#if defined(_WIN32)
        std::tm tm{};
        localtime_s(&tm, &t);
        return tm;
#else
        std::tm tm{};
        localtime_r(&t, &tm);
        return tm;
#endif
    }

   public:
    template <typename... Args>
    static void write(Level lvl, const std::source_location& loc, Args&&... args)
    {
        if (static_cast<uint8_t>(lvl) < static_cast<uint8_t>(min_level)) return;

        std::lock_guard lock(write_mutex);

        // timestamp
        const auto now = std::chrono::system_clock::now();
        const auto t = std::chrono::system_clock::to_time_t(now);
        const auto tm = local_time(t);

        std::ostringstream msg;
        (msg << ... << std::forward<Args>(args));

        std::cout << "[" << std::put_time(&tm, "%H:%M:%S") << "] " << level_color(lvl) << level_name(lvl) << " "
                  << msg.str();

#ifndef NDEBUG
        std::cout << " \033[90m(" << loc.file_name() << ":" << loc.line() << " " << loc.function_name() << ")\033[0m";
#endif

        std::cout << '\n';
    }

    template <typename Ex, typename... Args>
    [[noreturn]] static void log_throw(Args&&... args)
    {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));

        write(Level::Error, std::source_location::current(), oss.str());
        throw Ex(oss.str());
    }
};

#define TYPE_NAME(x) Log::type_name<x>()
#define LOG_TRACE(...) Log::write(Log::Level::Trace, std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...) Log::write(Log::Level::Debug, std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) Log::write(Log::Level::Info, std::source_location::current(), __VA_ARGS__)
#define LOG_WARN(...) Log::write(Log::Level::Warn, std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) Log::write(Log::Level::Error, std::source_location::current(), __VA_ARGS__)
#define LOG_CRITICAL(...) Log::write(Log::Level::Critical, std::source_location::current(), __VA_ARGS__)
#define LOG_THROW(type, ...) Log::log_throw<type>(__VA_ARGS__)
