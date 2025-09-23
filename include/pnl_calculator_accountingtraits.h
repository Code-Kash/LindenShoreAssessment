#pragma once

#include "pnl_calculator_constants.h"
#include "pnl_calculator_enums.h"
#include <type_traits>
#include <cmath>

namespace pnl::traits {

// Base accounting traits template
template<enums::AccountingType Method>
struct AccountingTraits {
    using method_type = std::integral_constant<enums::AccountingType, Method>;

    static constexpr enums::AccountingType accounting_method = Method;
    static constexpr bool is_fifo = (Method == enums::AccountingType::FIFO);
    static constexpr bool is_lifo = (Method == enums::AccountingType::LIFO);
    static constexpr int decimal_precision = constants::DEFAULT_DECIMAL_PRECISION;
    static constexpr double precision_multiplier = std::pow(10.0, decimal_precision);
    static constexpr std::size_t default_reserve_size = constants::DEFAULT_RESERVE_SIZE;
    static constexpr std::size_t cache_line_size = constants::CACHE_LINE_SIZE;

    // Compile-time precision formatting
    template<typename T>
    static constexpr double format_precision(T value) noexcept {
        return std::round(static_cast<double>(value) * precision_multiplier) / precision_multiplier;
    }
};

// FIFO specialization
template<>
struct AccountingTraits<enums::AccountingType::FIFO> {
    using method_type = std::integral_constant<enums::AccountingType, enums::AccountingType::FIFO>;

    static constexpr enums::AccountingType accounting_method = enums::AccountingType::FIFO;
    static constexpr bool is_fifo = true;
    static constexpr bool is_lifo = false;
    static constexpr int decimal_precision = constants::DEFAULT_DECIMAL_PRECISION;
    static constexpr double precision_multiplier = std::pow(10.0, decimal_precision);
    static constexpr std::size_t default_reserve_size = constants::DEFAULT_RESERVE_SIZE;
    static constexpr std::size_t cache_line_size = constants::CACHE_LINE_SIZE;

    template<typename T>
    static constexpr double format_precision(T value) noexcept {
        return std::round(static_cast<double>(value) * precision_multiplier) / precision_multiplier;
    }

    // FIFO-specific optimization: use front of container
    static constexpr bool use_front_access = true;
    static constexpr bool reverse_iteration = false;
};

// LIFO specialization
template<>
struct AccountingTraits<enums::AccountingType::LIFO> {
    using method_type = std::integral_constant<enums::AccountingType, enums::AccountingType::LIFO>;

    static constexpr enums::AccountingType accounting_method = enums::AccountingType::LIFO;
    static constexpr bool is_fifo = false;
    static constexpr bool is_lifo = true;
    static constexpr int decimal_precision = constants::DEFAULT_DECIMAL_PRECISION;
    static constexpr double precision_multiplier = std::pow(10.0, decimal_precision);
    static constexpr std::size_t default_reserve_size = constants::DEFAULT_RESERVE_SIZE;
    static constexpr std::size_t cache_line_size = constants::CACHE_LINE_SIZE;

    template<typename T>
    static constexpr double format_precision(T value) noexcept {
        return std::round(static_cast<double>(value) * precision_multiplier) / precision_multiplier;
    }

    // LIFO-specific optimization: use back of container
    static constexpr bool use_front_access = false;
    static constexpr bool reverse_iteration = true;
};

// Type selector for runtime accounting method selection
template<typename F, typename L>
constexpr auto select_accounting_traits(enums::AccountingType method, F&& fifo_func, L&& lifo_func) {
    if constexpr (std::is_invocable_v<F> && std::is_invocable_v<L>) {
        switch (method) {
            case enums::AccountingType::FIFO:
                return std::forward<F>(fifo_func)();
            case enums::AccountingType::LIFO:
                return std::forward<L>(lifo_func)();
        }
    }
    return std::forward<F>(fifo_func)(); // default to FIFO
}

// Compile-time method selection via type traits
template<enums::AccountingType Method>
using accounting_traits_t = AccountingTraits<Method>;

using fifo_traits = AccountingTraits<enums::AccountingType::FIFO>;
using lifo_traits = AccountingTraits<enums::AccountingType::LIFO>;

} // namespace pnl::traits