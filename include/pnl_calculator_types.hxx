#pragma once

#include <sstream>
#include <stdexcept>
#include <vector>
#include <iomanip>

namespace pnl::types
{
    inline Trade::Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, enums::TradeSide s)
        : timestamp_(ts), symbol_(std::move(sym)), price_(p), quantity_(q), side_(s)
    {}

    inline Trade::Trade(timestamp_t ts, symbol_t sym, price_t p, quantity_t q, char side_char)
        : timestamp_(ts), symbol_(std::move(sym)), price_(p), quantity_(q),
          side_(side_char == constants::BUY_INDICATOR ? enums::TradeSide::BUY : enums::TradeSide::SELL)
    {}

    inline constexpr bool Trade::is_buy() const noexcept
    {
        return side_ == enums::TradeSide::BUY;
    }

    inline constexpr bool Trade::is_sell() const noexcept
    {
        return side_ == enums::TradeSide::SELL;
    }

    inline Trade Trade::parse(const std::string& csv_line)
    {
        std::istringstream iss(csv_line);
        std::string token;
        std::vector<std::string> tokens;
        tokens.reserve(5);

        while (std::getline(iss, token, constants::CSV_DELIMITER))
        {
            tokens.emplace_back(std::move(token));
        }

        if (tokens.size() != 5)
        {
            throw std::invalid_argument("Invalid CSV format: expected 5 fields, got " + std::to_string(tokens.size()));
        }

        try
        {
            auto timestamp = std::stoull(tokens[0]);
            auto symbol = std::move(tokens[1]);
            auto side_char = tokens[2].empty() ? '\0' : tokens[2][0];
            auto price_d = std::stod(tokens[3]);
            auto quantity_d = std::stod(tokens[4]);

            if (side_char != constants::BUY_INDICATOR && side_char != constants::SELL_INDICATOR)
            {
                throw std::invalid_argument("Invalid trade side: " + tokens[2]);
            }

            if (price_d <= 0.0 || quantity_d <= 0.0)
            {
                throw std::invalid_argument("Price and quantity must be positive");
            }

            auto price = price_d;
            auto quantity = static_cast<quantity_t>(quantity_d);

            return Trade{timestamp, std::move(symbol), price, quantity, side_char};
        }
        catch (const std::invalid_argument& e)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Parse error: " + std::string(e.what()));
        }
    }

    inline std::string Trade::to_string() const
    {
        std::ostringstream oss;
        oss << "Trade{timestamp=" << timestamp_
            << ", symbol=" << symbol_
            << ", price=" << price_
            << ", quantity=" << quantity_
            << ", side=" << utils::trade_side_to_string(side_)
            << "}";
        return oss.str();
    }

    inline constexpr Position::Position(price_t p, quantity_t q, timestamp_t ts) noexcept
        : price_(p), quantity_(q), timestamp_(ts)
    {}

    inline constexpr bool Position::is_empty() const noexcept
    {
        return quantity_ == 0;
    }

    inline constexpr void Position::reduce_quantity(quantity_t amount) noexcept
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

    inline PnLResult::PnLResult(timestamp_t ts, symbol_t sym, pnl_t p)
        : timestamp_(ts), symbol_(std::move(sym)), pnl_(p)
    {}

    template <int Precision>
    inline std::string PnLResult::to_csv_string() const
    {
        std::ostringstream oss;
        oss << timestamp_ << constants::CSV_DELIMITER
            << symbol_ << constants::CSV_DELIMITER
            << std::fixed << std::setprecision(Precision) << pnl_;
        return oss.str();
    }

    inline ErrorResult::ErrorResult(enums::ErrorType type, std::string msg, int code)
        : type_(type), message_(std::move(msg)), error_code_(code)
    {}

    inline std::string ErrorResult::to_string() const
    {
        std::ostringstream oss;
        oss << "ErrorResult{type=" << static_cast<int>(type_)
            << ", message=" << message_
            << ", code=" << error_code_
            << "}";
        return oss.str();
    }
}