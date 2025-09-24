#pragma once

#include "pnl_calculator_enums.h"
#include "pnl_calculator_constants.h"
#include "pnl_calculator_utils.h"
#include "pnl_calculator_accountingtraits.h"
#include "pnl_calculator_macros.h"
#include "pnl_calculator_concepts.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace pnl::types
{
    using namespace traits;

    using timestamp_t = AccountingTraitsBase::timestamp_t;
    using price_t = AccountingTraitsBase::price_t;
    using quantity_t = AccountingTraitsBase::quantity_t;
    using symbol_t = AccountingTraitsBase::symbol_t;
    using pnl_t = AccountingTraitsBase::pnl_t;

    struct Trade;
    struct Position;
    struct PnLResult;
    struct ErrorResult;

    struct CACHE_LINE_ALIGNED Trade
    {
    private:
        timestamp_t timestamp_;
        symbol_t symbol_;
        price_t price_;
        quantity_t quantity_;
        enums::TradeSide side_;

    public:
        RULE_OF_FIVE_COPYABLE(Trade)

        Trade() = default;

        Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, enums::TradeSide s);
        Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, char side_char);

        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }
        [[nodiscard]] const symbol_t& symbol() const noexcept { return symbol_; }
        [[nodiscard]] constexpr price_t price() const noexcept { return price_; }
        [[nodiscard]] constexpr quantity_t quantity() const noexcept { return quantity_; }
        [[nodiscard]] constexpr enums::TradeSide side() const noexcept { return side_; }

        [[nodiscard]] constexpr bool is_buy() const noexcept;
        [[nodiscard]] constexpr bool is_sell() const noexcept;
        static Trade parse(const std::string& csv_line);
        [[nodiscard]] std::string to_string() const;
    };

    struct CACHE_LINE_ALIGNED Position
    {
    private:
        price_t price_;
        quantity_t quantity_;
        timestamp_t timestamp_;

    public:
        RULE_OF_FIVE_COPYABLE(Position)

        Position() = default;

        constexpr Position(price_t p, quantity_t q, timestamp_t ts) noexcept;

        [[nodiscard]] constexpr price_t price() const noexcept { return price_; }
        [[nodiscard]] constexpr quantity_t quantity() const noexcept { return quantity_; }
        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }

        [[nodiscard]] constexpr bool is_empty() const noexcept;
        constexpr void reduce_quantity(quantity_t amount) noexcept;
    };

    struct CACHE_LINE_ALIGNED PnLResult
    {
    private:
        timestamp_t timestamp_;
        symbol_t symbol_;
        pnl_t pnl_;

    public:
        RULE_OF_FIVE_COPYABLE(PnLResult)

        PnLResult() = default;

        PnLResult(timestamp_t ts, symbol_t sym, pnl_t p);

        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }
        [[nodiscard]] const symbol_t& symbol() const noexcept { return symbol_; }
        [[nodiscard]] constexpr pnl_t pnl() const noexcept { return pnl_; }

        template <int Precision = constants::DEFAULT_DECIMAL_PRECISION>
        [[nodiscard]] std::string to_csv_string() const;
    };

    struct CACHE_LINE_ALIGNED ErrorResult
    {
    private:
        enums::ErrorType type_;
        std::string message_;
        int error_code_;

    public:
        RULE_OF_FIVE_COPYABLE(ErrorResult)

        ErrorResult() = default;

        ErrorResult(enums::ErrorType type, std::string msg, int code);

        [[nodiscard]] constexpr enums::ErrorType type() const noexcept { return type_; }
        [[nodiscard]] const std::string& message() const noexcept { return message_; }
        [[nodiscard]] constexpr int error_code() const noexcept { return error_code_; }

        [[nodiscard]] std::string to_string() const;
    };

    // Type alias for convenience
    template <enums::AccountingType Method>
    using accounting_method_t = std::integral_constant<enums::AccountingType, Method>;
}

#include "pnl_calculator_types.hxx"