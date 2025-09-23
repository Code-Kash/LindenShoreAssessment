#pragma once

#include <cstddef>
#include <limits>

namespace pnl::constants
{
    constexpr int DEFAULT_DECIMAL_PRECISION = 2;
    constexpr double EPSILON = 1e-9;

    constexpr std::size_t DEFAULT_RESERVE_SIZE = 1024;
    constexpr std::size_t CACHE_LINE_SIZE = 64;
    constexpr std::size_t MAX_SYMBOL_LENGTH = 16;

    constexpr char CSV_DELIMITER = ',';
    constexpr char BUY_INDICATOR = 'B';
    constexpr char SELL_INDICATOR = 'S';

    constexpr const char* CSV_HEADER = "timestamp,symbol,pnl";
    constexpr const char* FIFO_ARG = "fifo";
    constexpr const char* LIFO_ARG = "lifo";

    constexpr int SUCCESS = 0;
    constexpr int ERROR_INVALID_ARGS = 1;
    constexpr int ERROR_FILE_NOT_FOUND = 2;
    constexpr int ERROR_PARSE_ERROR = 3;
    constexpr int ERROR_INVALID_ACCOUNTING = 4;
}