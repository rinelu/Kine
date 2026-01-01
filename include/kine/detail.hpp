#pragma once
#include <string_view>

namespace kine::detail
{

template <typename T>
constexpr std::string_view wrapped_type_name()
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

constexpr std::string_view extract(std::string_view wrapped)
{
#if defined(__clang__)
    auto start = wrapped.find("T = ") + 4;
    auto end = wrapped.rfind(']');
    return wrapped.substr(start, end - start);
#elif defined(__GNUC__)
    auto start = wrapped.find("T = ") + 4;
    auto end = wrapped.rfind(']');
    return wrapped.substr(start, end - start);
#elif defined(_MSC_VER)
    auto start = wrapped.find("wrapped_type_name<") + 18;
    auto end = wrapped.find(">(void)");
    return wrapped.substr(start, end - start);
#endif
}

template <typename T>
constexpr const char* type_name()
{
    return extract(wrapped_type_name<T>()).data();
}

}  // namespace kine::detail

#define TYPE_NAME(value) kine::detail::type_name<value>()
