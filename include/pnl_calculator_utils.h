#pragma once

#include "pnl_calculator_enums.h"
#include <string_view>

namespace pnl::utils
{
    constexpr enums::AccountingType string_to_accounting_type(std::string_view str) noexcept
    {
        if (str == "fifo") return enums::AccountingType::FIFO;
        if (str == "lifo") return enums::AccountingType::LIFO;
        return enums::AccountingType::FIFO; // default
    }

    constexpr enums::TradeSide char_to_trade_side(char c) noexcept
    {
        return (c == 'B' || c == 'b') ? enums::TradeSide::BUY : enums::TradeSide::SELL;
    }

    constexpr const char* accounting_type_to_string(enums::AccountingType type) noexcept
    {
        switch (type)
        {
            case enums::AccountingType::FIFO: return "FIFO";
            case enums::AccountingType::LIFO: return "LIFO";
            default: return "UNKNOWN";
        }
    }

    constexpr const char* trade_side_to_string(enums::TradeSide side) noexcept
    {
        switch (side)
        {
            case enums::TradeSide::BUY: return "BUY";
            case enums::TradeSide::SELL: return "SELL";
            default: return "UNKNOWN";
        }
    }
}