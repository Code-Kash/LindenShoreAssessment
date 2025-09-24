#pragma once

#include <concepts>
#include <string>
#include <type_traits>

namespace pnl::concepts
{
    template <typename T>
    concept Arithmetic = std::floating_point<T> || std::integral<T>;

    template <typename T>
    concept StringLike = requires(T t)
    {
        { t.c_str() } -> std::convertible_to<const char*>;
        { t.size() } -> std::convertible_to<std::size_t>;
        { t.empty() } -> std::convertible_to<bool>;
    } || std::same_as<T, const char*> || std::same_as<T, char*>;

    template <typename T>
    concept AccountingMethod = requires
    {
        typename T::method_type;
        { T::is_fifo } -> std::convertible_to<bool>;
        { T::is_lifo } -> std::convertible_to<bool>;
        { T::decimal_precision } -> std::convertible_to<int>;
    };

    template <typename T>
    concept Trade = requires(T t)
    {
        requires Arithmetic<decltype(t.timestamp())>;
        requires StringLike<decltype(t.symbol())>;
        { t.is_buy() } -> std::convertible_to<bool>;
        requires Arithmetic<decltype(t.price())>;
        requires Arithmetic<decltype(t.quantity())>;
    };

    template <typename T>
    concept Position = requires(T t)
    {
        requires Arithmetic<decltype(t.price())>;
        requires Arithmetic<decltype(t.quantity())>;
        requires Arithmetic<decltype(t.timestamp())>;
    };

    template <typename T>
    concept TradeContainer = requires(T t)
    {
        typename T::value_type;
        typename T::iterator;
        typename T::const_iterator;
        { t.begin() } -> std::same_as<typename T::iterator>;
        { t.end() } -> std::same_as<typename T::iterator>;
        { t.size() } -> std::convertible_to<std::size_t>;
        { t.empty() } -> std::convertible_to<bool>;
        requires Trade<typename T::value_type>;
    };

    template <typename T>
    concept PnLResult = requires(T t)
    {
        requires Arithmetic<decltype(t.timestamp())>;
        requires StringLike<decltype(t.symbol())>;
        requires Arithmetic<decltype(t.pnl())>;
    };

    template <typename T>
    concept CSVParseable = requires(T t, const std::string& line)
    {
        { T::parse(line) } -> std::same_as<T>;
    };

    template <typename T>
    concept Formattable = requires(T t)
    {
        { t.to_csv_string() } -> std::convertible_to<std::string>;
    };
}