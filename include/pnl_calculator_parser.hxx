#pragma once

#include <sstream>
#include <fstream>

namespace pnl::parser
{
    inline std::vector<std::string> CSVParser::split_csv_line(const std::string& line)
    {
        std::vector<std::string> tokens;
        tokens.reserve(5);

        std::string token;
        bool in_quotes = false;

        for (char c : line)
        {
            if (c == '"') UNLIKELY
            {
                in_quotes = !in_quotes;
            }
            else if (c == constants::CSV_DELIMITER && !in_quotes) LIKELY
            {
                tokens.emplace_back(std::move(token));
                token.clear();
            }
            else
            {
                token += c;
            }
        }

        if (!token.empty())
        {
            tokens.emplace_back(std::move(token));
        }

        return tokens;
    }

    inline Result<types::Trade, types::ErrorResult> CSVParser::parse_trade_line(const std::string& line)
    {
        if (line.empty() || line[0] == '#') UNLIKELY
        {
            return Result<types::Trade, types::ErrorResult>::error(
                enums::ErrorType::PARSE_ERROR,
                "Empty or comment line",
                constants::ERROR_PARSE_ERROR
            );
        }

        auto tokens = split_csv_line(line);

        if (tokens.size() != 5) UNLIKELY
        {
            return Result<types::Trade, types::ErrorResult>::error(
                enums::ErrorType::PARSE_ERROR,
                "Invalid number of CSV fields: expected 5, got " + std::to_string(tokens.size()),
                constants::ERROR_PARSE_ERROR
            );
        }

        try
        {
            auto timestamp = std::stoull(tokens[0]);
            auto symbol = std::move(tokens[1]);
            auto side_char = tokens[2].empty() ? '\0' : tokens[2][0];
            auto price_d = std::stod(tokens[3]);
            auto quantity_d = std::stod(tokens[4]);

            if (side_char != constants::BUY_INDICATOR && side_char != constants::SELL_INDICATOR) UNLIKELY
            {
                return Result<types::Trade, types::ErrorResult>::error(
                    enums::ErrorType::INVALID_TRADE_DATA,
                    "Invalid trade side: " + tokens[2],
                    constants::ERROR_PARSE_ERROR
                );
            }

            if (price_d <= 0.0 || quantity_d <= 0.0) UNLIKELY
            {
                return Result<types::Trade, types::ErrorResult>::error(
                    enums::ErrorType::INVALID_TRADE_DATA,
                    "Invalid price or quantity: must be positive",
                    constants::ERROR_PARSE_ERROR
                );
            }

            auto price = price_d;
            auto quantity = static_cast<types::quantity_t>(quantity_d);

            return types::Trade{timestamp, std::move(symbol), price, quantity, side_char};
        }
        catch (const std::exception& e)
        {
            return Result<types::Trade, types::ErrorResult>::error(
                enums::ErrorType::PARSE_ERROR,
                "Parse error: " + std::string(e.what()),
                constants::ERROR_PARSE_ERROR
            );
        }
    }

    template <concepts::StringLike Path>
    inline std::optional<std::vector<types::Trade>> CSVParser::parse_file(const Path& filename)
    {
        std::ifstream file;

        if constexpr (std::is_same_v<Path, std::string>)
        {
            file.open(filename);
        }
        else
        {
            file.open(std::string(filename));
        }

        if (!file) [[unlikely]]
        {
            return std::nullopt;
        }

        std::vector<types::Trade> trades;
        trades.reserve(constants::DEFAULT_RESERVE_SIZE);

        std::string line;
        std::size_t line_number = 0;

        while (std::getline(file, line))
        {
            ++line_number;

            if (line.empty() || line[0] == '#') [[unlikely]]
            {
                continue;
            }

            auto result = parse_trade_line(line);
            if (result.has_value()) [[likely]]
            {
                trades.emplace_back(std::move(result.value()));
            }
            else
            {
                continue;
            }
        }

        return trades;
    }

    template <typename Stream>
    requires requires(Stream& s)
    {
        { std::getline(s, std::declval<std::string&>()) };
        { s.good() } -> std::convertible_to<bool>;
    }
    inline typename CSVParser::ParseResult CSVParser::parse(Stream& stream)
    {
        std::vector<types::Trade> trades;
        trades.reserve(constants::DEFAULT_RESERVE_SIZE);

        std::string line;
        std::size_t line_number = 0;

        while (std::getline(stream, line))
        {
            ++line_number;

            if (line.empty() || line[0] == '#') [[unlikely]]
            {
                continue;
            }

            auto result = parse_trade_line(line);
            if (result.has_value()) [[likely]]
            {
                trades.emplace_back(std::move(result.value()));
            }
            else
            {
                const auto& error = result.error();
                return ParseResult::error(
                    error.type(),
                    error.message() + " (line " + std::to_string(line_number) + ")",
                    error.error_code()
                );
            }
        }

        if (trades.empty()) [[unlikely]]
        {
            return ParseResult::error(
                enums::ErrorType::PARSE_ERROR,
                "No valid trades found in stream",
                constants::ERROR_PARSE_ERROR
            );
        }

        return ParseResult::success(std::move(trades));
    }
}