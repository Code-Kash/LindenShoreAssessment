#pragma once

#include "pnl_calculator_concepts.h"
#include "pnl_calculator_constants.h"
#include "pnl_calculator_enums.h"
#include "pnl_calculator_macros.h"
#include "pnl_calculator_accountingtraits.h"
#include "pnl_calculator_utils.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace pnl::types
{
    using timestamp_t = traits::AccountingTraitsBase::timestamp_t;
    using price_t = traits::AccountingTraitsBase::price_t;
    using quantity_t = traits::AccountingTraitsBase::quantity_t;
    using symbol_t = traits::AccountingTraitsBase::symbol_t;
    using pnl_t = traits::AccountingTraitsBase::pnl_t;

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

        Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, enums::TradeSide s)
            : timestamp_(ts), symbol_(std::move(sym)), price_(p), quantity_(q), side_(s)
        {}

        Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, char side_char)
            : timestamp_(ts), symbol_(std::move(sym)), price_(p), quantity_(q),
              side_(utils::char_to_trade_side(side_char))
        {}

        // Accessor methods
        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }
        [[nodiscard]] const symbol_t& symbol() const noexcept { return symbol_; }
        [[nodiscard]] constexpr price_t price() const noexcept { return price_; }
        [[nodiscard]] constexpr quantity_t quantity() const noexcept { return quantity_; }
        [[nodiscard]] constexpr enums::TradeSide side() const noexcept { return side_; }

        [[nodiscard]] constexpr bool is_buy() const noexcept
    	{
            return side_ == enums::TradeSide::BUY;
        }

        [[nodiscard]] constexpr bool is_sell() const noexcept
    	{
            return side_ == enums::TradeSide::SELL;
        }

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

        Position(price_t p, quantity_t q, timestamp_t ts)
            : price_(p), quantity_(q), timestamp_(ts)
        {}

        [[nodiscard]] constexpr price_t price() const noexcept { return price_; }
        [[nodiscard]] constexpr quantity_t quantity() const noexcept { return quantity_; }
        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }

        [[nodiscard]] constexpr bool is_empty() const noexcept
    	{
            return quantity_ == 0;
        }

        constexpr void reduce_quantity(quantity_t amount) noexcept
    	{
            if (amount >= quantity_)
    		{
                quantity_ = 0;
            }
            else
    		{
                quantity_ -= amount;
            }
        }
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

        PnLResult(timestamp_t ts, symbol_t sym, pnl_t p)
            : timestamp_(ts), symbol_(std::move(sym)), pnl_(p)
        {}

        [[nodiscard]] constexpr timestamp_t timestamp() const noexcept { return timestamp_; }
        [[nodiscard]] const symbol_t& symbol() const noexcept { return symbol_; }
        [[nodiscard]] constexpr pnl_t pnl() const noexcept { return pnl_; }

        template <int Precision = constants::DEFAULT_DECIMAL_PRECISION>
        [[nodiscard]] std::string to_csv_string() const
    	{
            std::ostringstream oss;
            oss << timestamp_ << constants::CSV_DELIMITER
                << symbol_ << constants::CSV_DELIMITER
                << std::fixed << std::setprecision(Precision) << pnl_;
            return oss.str();
        }
    };

    struct ErrorResult
    {
    private:
        enums::ErrorType type_;
        std::string message_;
        int error_code_;

    public:
        RULE_OF_FIVE_COPYABLE(ErrorResult)

        ErrorResult() : type_(enums::ErrorType::NONE), error_code_(constants::SUCCESS) {}

        ErrorResult(enums::ErrorType t, std::string msg, int code = constants::ERROR_PARSE_ERROR)
            : type_(t), message_(std::move(msg)), error_code_(code)
        {}

        [[nodiscard]] constexpr enums::ErrorType type() const noexcept { return type_; }
        [[nodiscard]] const std::string& message() const noexcept { return message_; }
        [[nodiscard]] constexpr int error_code() const noexcept { return error_code_; }

        [[nodiscard]] constexpr bool has_error() const noexcept
    	{
            return type_ != enums::ErrorType::NONE;
        }

        [[nodiscard]] std::string to_string() const;
    };

    template <enums::AccountingType Method>
    using accounting_method_t = std::integral_constant<enums::AccountingType, Method>;
}

static_assert(pnl::concepts::Trade<pnl::types::Trade>);
static_assert(pnl::concepts::Position<pnl::types::Position>);
static_assert(pnl::concepts::PnLResult<pnl::types::PnLResult>);
static_assert(pnl::concepts::Formattable<pnl::types::PnLResult>);