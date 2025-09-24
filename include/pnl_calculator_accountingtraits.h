#pragma once

#include "pnl_calculator_constants.h"
#include "pnl_calculator_enums.h"
#include <type_traits>
#include <cmath>
#include <cstdint>
#include <string>

namespace pnl::traits
{
    struct AccountingTraitsBase
    {
        using timestamp_t = std::uint64_t;
        using price_t = double;
        using quantity_t = std::uint32_t;
        using symbol_t = std::string;
        using pnl_t = double;

        static constexpr int decimal_precision = constants::DEFAULT_DECIMAL_PRECISION;
        static constexpr double precision_multiplier = std::pow(10.0, decimal_precision);
        static constexpr std::size_t default_reserve_size = constants::DEFAULT_RESERVE_SIZE;
        static constexpr std::size_t cache_line_size = constants::CACHE_LINE_SIZE;

        template <typename T>
        static constexpr double format_precision(T value) noexcept
    	{
            return std::round(static_cast<double>(value) * precision_multiplier) / precision_multiplier;
        }
    };

    template <enums::AccountingType Method>
    struct AccountingTraits : AccountingTraitsBase
    {
        using method_type = std::integral_constant<enums::AccountingType, Method>;

        static constexpr enums::AccountingType accounting_method = Method;
        static constexpr bool is_fifo = (Method == enums::AccountingType::FIFO);
        static constexpr bool is_lifo = (Method == enums::AccountingType::LIFO);
    };

    template <>
    struct AccountingTraits<enums::AccountingType::FIFO> : AccountingTraitsBase
    {
        using method_type = std::integral_constant<enums::AccountingType, enums::AccountingType::FIFO>;

        static constexpr enums::AccountingType accounting_method = enums::AccountingType::FIFO;
        static constexpr bool is_fifo = true;
        static constexpr bool is_lifo = false;

        static constexpr bool use_front_access = true;
        static constexpr bool reverse_iteration = false;
    };

    template <>
    struct AccountingTraits<enums::AccountingType::LIFO> : AccountingTraitsBase
    {
        using method_type = std::integral_constant<enums::AccountingType, enums::AccountingType::LIFO>;

        static constexpr enums::AccountingType accounting_method = enums::AccountingType::LIFO;
        static constexpr bool is_fifo = false;
        static constexpr bool is_lifo = true;

        static constexpr bool use_front_access = false;
        static constexpr bool reverse_iteration = true;
    };
}