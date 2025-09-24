#pragma once

#include <string_view>
#include <type_traits>

namespace pnl::enums
{
    enum class AccountingType : uint8_t
     {
        FIFO = 0,
        LIFO = 1
    };

    enum class TradeSide : uint8_t
     {
        BUY = 0,
        SELL = 1
    };

    enum class ErrorType : uint8_t
     {
        NONE = 0,
        INVALID_ARGUMENTS,
        FILE_NOT_FOUND,
        PARSE_ERROR,
        INVALID_ACCOUNTING_METHOD,
        INSUFFICIENT_POSITI,
        INVALID_TRADE_DATA
    };
}