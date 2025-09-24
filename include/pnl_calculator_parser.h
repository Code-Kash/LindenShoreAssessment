#pragma once

#include "pnl_calculator_concepts.h"
#include "pnl_calculator_types.h"
#include "pnl_calculator_constants.h"
#include "pnl_calculator_macros.h"
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <variant>
#include <optional>

// Result type for error handling (C++20 compatible alternative to C++23's std::expected)
template <typename T, typename E>
class Result
{
private:
    std::variant<T, E> data_;

public:
    Result(T&& value) : data_(std::forward<T>(value)) {}
    Result(E&& error) : data_(std::forward<E>(error)) {}

    template <typename... Args>
    static Result success(Args&&... args) 
    {
        return Result{T{std::forward<Args>(args)...}};
    }

    template <typename... Args>
    static Result error(Args&&... args) 
    {
        return Result{E{std::forward<Args>(args)...}};
    }

    bool has_value() const noexcept { return std::holds_alternative<T>(data_); }
    bool has_error() const noexcept { return std::holds_alternative<E>(data_); }

    T& value() & { return std::get<T>(data_); }
    const T& value() const& { return std::get<T>(data_); }
    T&& value() && { return std::get<T>(std::move(data_)); }

    E& error() & { return std::get<E>(data_); }
    const E& error() const& { return std::get<E>(data_); }
    E&& error() && { return std::get<E>(std::move(data_)); }

    bool is_success() const noexcept { return has_value(); }
    bool is_error() const noexcept { return has_error(); }

    explicit operator bool() const noexcept { return has_value(); }
};

namespace pnl::parser
{
    class CSVParser
    {
    public:
        RULE_OF_FIVE_NONMOVABLE(CSVParser)

        using ParseResult = Result<std::vector<types::Trade>, types::ErrorResult>;

        FORCE_INLINE static std::vector<std::string> split_csv_line(const std::string& line);
        static Result<types::Trade, types::ErrorResult> parse_trade_line(const std::string& line);

        template <concepts::StringLike Path>
        [[nodiscard]] static std::optional<std::vector<types::Trade>> parse_file(const Path& filename);

        template <typename Stream>
        requires requires(Stream& s) 
        {
            { std::getline(s, std::declval<std::string&>()) };
            { s.good() } -> std::convertible_to<bool>;
        }
        [[nodiscard]] ParseResult parse(Stream& stream);
    };
}

#include "pnl_calculator_parser.hxx"