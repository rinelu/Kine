#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <source_location>
#include <sstream>
#include <string_view>

#define UNUSED(a) (void)a;
#define NDEBUG

class Log
{
   public:
    enum class Level
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    static inline bool use_color = true;
    static inline Level min_level =
#ifdef NDEBUG
        Level::Info;
#else
        Level::Trace;
#endif

   private:
    static inline std::mutex write_mutex;

    static const std::string_view level_name(Level lvl)
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

    static const char* level_color(Level lvl)
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

    static void reset_color()
    {
        if (use_color) std::cout << "\033[0m";
    }

   public:
    template <typename... Args>
    static void write(Level lvl, const std::source_location& loc, Args&&... args)
    {
        if (lvl < min_level) return;

        std::lock_guard<std::mutex> lock(write_mutex);

        // timestamp
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&t);

        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%H:%M:%S");

        // message
        std::ostringstream msg;
        (msg << ... << args);

        // output
        std::cout << level_color(lvl) << "[" << timestamp.str() << "] " << level_name(lvl) << "\033[0m" << " " << msg.str();

#ifndef NDEBUG
        std::cout << "  \033[90m"
                  << "(" << loc.file_name() << ":" << loc.line() << " " << loc.function_name() << ")"
                  << "\033[0m";
#endif
        UNUSED(loc);

        reset_color();
        std::cout << std::endl;
    }

    template <typename Ex, typename... Args>
    static void log_throw(Args&&... args)
    {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        Log::write(Log::Level::Error, std::source_location::current(), oss.str());
        throw Ex(oss.str());
    }
};

#define LOG_TRACE(...) Log::write(Log::Level::Trace, std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(...) Log::write(Log::Level::Debug, std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) Log::write(Log::Level::Info, std::source_location::current(), __VA_ARGS__)
#define LOG_WARN(...) Log::write(Log::Level::Warn, std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) Log::write(Log::Level::Error, std::source_location::current(), __VA_ARGS__)
#define LOG_CRITICAL(...) Log::write(Log::Level::Critical, std::source_location::current(), __VA_ARGS__)
#define LOG_THROW(exType, ...) Log::log_throw<exType>(__VA_ARGS__)
