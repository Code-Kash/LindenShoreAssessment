#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include "include/pnl_calculator_types.h"
#include "include/pnl_calculator_parser.h"
#include "include/pnl_calculator_engine.h"

using namespace pnl;

void test_types()
{
    std::cout << "Testing Types..." << std::endl;

    types::Trade trade{1000000000, "AAPL", 150.25, 100, enums::TradeSide::BUY};
    assert(trade.timestamp() == 1000000000);
    assert(trade.symbol() == "AAPL");
    assert(std::abs(trade.price() - 150.25) < 0.001);
    assert(trade.quantity() == 100);
    assert(trade.side() == enums::TradeSide::BUY);
    assert(trade.is_buy() == true);
    assert(!trade.is_sell());

    types::Position pos{150.25, 100, 1000000000};
    assert(std::abs(pos.price() - 150.25) < 0.001);
    assert(pos.quantity() == 100);
    assert(pos.timestamp() == 1000000000);
    assert(!pos.is_empty());

    pos.reduce_quantity(50);
    assert(pos.quantity() == 50);
    assert(!pos.is_empty());

    pos.reduce_quantity(50);
    assert(pos.quantity() == 0);
    assert(pos.is_empty());

    std::cout << "  ✓ Type tests passed" << std::endl;
}

void test_parser()
{
    std::cout << "Testing Parser..." << std::endl;

    std::string line1 = "1000000000,AAPL,B,150.25,100";
    auto result1 = parser::CSVParser::parse_trade_line(line1);
    assert(result1.has_value());
    const auto& trade1 = result1.value();
    assert(trade1.timestamp() == 1000000000);
    assert(trade1.symbol() == "AAPL");
    assert(trade1.is_buy());
    assert(std::abs(trade1.price() - 150.25) < 0.001);
    assert(trade1.quantity() == 100);

    std::string line2 = "1000000001,AAPL,S,151.00,50";
    auto result2 = parser::CSVParser::parse_trade_line(line2);
    assert(result2.has_value());
    const auto& trade2 = result2.value();
    assert(trade2.timestamp() == 1000000001);
    assert(trade2.is_sell());
    assert(std::abs(trade2.price() - 151.00) < 0.001);
    assert(trade2.quantity() == 50);

    std::string invalid = "invalid,data";
    auto result3 = parser::CSVParser::parse_trade_line(invalid);
    assert(result3.has_error());
    assert(result3.error().type() == enums::ErrorType::PARSE_ERROR);

    std::cout << "  ✓ Parser tests passed" << std::endl;
}

void test_engine_basic()
{
    std::cout << "Testing Engine..." << std::endl;

    auto fifo_engine = engine::create_engine<enums::AccountingType::FIFO>();
    std::vector<types::Trade> trades = {
        types::Trade{1000000000, "AAPL", 150.00, 100, enums::TradeSide::BUY},
        types::Trade{1000000001, "AAPL", 151.00, 100, enums::TradeSide::SELL}
    };

    fifo_engine.process_trades(trades);
    const auto& fifo_results = fifo_engine.get_results();
    assert(fifo_results.size() == 1);
    assert(fifo_results[0].timestamp() == 1000000001);
    assert(fifo_results[0].symbol() == "AAPL");
    assert(std::abs(fifo_results[0].pnl() - 100.0) < 0.01);

    auto lifo_engine = engine::create_engine<enums::AccountingType::LIFO>();
    lifo_engine.process_trades(trades);
    const auto& lifo_results = lifo_engine.get_results();
    assert(lifo_results.size() == 1);
    assert(std::abs(lifo_results[0].pnl() - 100.0) < 0.01);

    std::cout << "  ✓ Engine tests passed" << std::endl;
}

void test_partial_fills()
{
    std::cout << "Testing Partial Fills..." << std::endl;

    auto engine = engine::create_engine<enums::AccountingType::FIFO>();
    std::vector<types::Trade> trades = {
        types::Trade{1000000000, "AAPL", 150.00, 100, enums::TradeSide::BUY},
        types::Trade{1000000001, "AAPL", 151.00, 50, enums::TradeSide::SELL},
        types::Trade{1000000002, "AAPL", 152.00, 50, enums::TradeSide::SELL}
    };

    engine.process_trades(trades);
    const auto& results = engine.get_results();
    assert(results.size() == 2);
    assert(std::abs(results[0].pnl() - 50.0) < 0.01);
    assert(std::abs(results[1].pnl() - 100.0) < 0.01);

    std::cout << "  ✓ Partial fill tests passed" << std::endl;
}

void test_multiple_symbols()
{
    std::cout << "Testing Multiple Symbols..." << std::endl;

    auto engine = engine::create_engine<enums::AccountingType::FIFO>();
    std::vector<types::Trade> trades = {
        types::Trade{1000000000, "AAPL", 150.00, 100, enums::TradeSide::BUY},
        types::Trade{1000000001, "GOOGL", 140.00, 50, enums::TradeSide::BUY},
        types::Trade{1000000002, "AAPL", 151.00, 100, enums::TradeSide::SELL},
        types::Trade{1000000003, "GOOGL", 141.00, 50, enums::TradeSide::SELL}
    };

    engine.process_trades(trades);
    const auto& results = engine.get_results();
    assert(results.size() == 2);

    for (const auto& result : results)
    {
        if (result.symbol() == "AAPL")
        {
            assert(std::abs(result.pnl() - 100.0) < 0.01);
        }
        else if (result.symbol() == "GOOGL")
        {
            assert(std::abs(result.pnl() - 50.0) < 0.01);
        }
    }

    std::cout << "  ✓ Multiple symbol tests passed" << std::endl;
}

void test_short_selling()
{
    std::cout << "Testing Short Selling..." << std::endl;

    auto engine = engine::create_engine<enums::AccountingType::FIFO>();
    std::vector<types::Trade> trades = {
        types::Trade{1000000000, "AAPL", 151.00, 100, enums::TradeSide::SELL},
        types::Trade{1000000001, "AAPL", 150.00, 100, enums::TradeSide::BUY}
    };

    engine.process_trades(trades);
    const auto& results = engine.get_results();
    assert(results.size() == 1);
    assert(std::abs(results[0].pnl() - 100.0) < 0.01);

    std::cout << "  ✓ Short selling tests passed" << std::endl;
}

void test_with_file()
{
    std::cout << "Testing with test_data.csv..." << std::endl;

    auto fifo_result = parser::CSVParser::parse_file(std::string("test_data.csv"));
    assert(fifo_result);
    assert(fifo_result.value().size() == 20);

    auto fifo_engine = engine::create_engine<enums::AccountingType::FIFO>();
    fifo_engine.process_trades(fifo_result.value());
    const auto& fifo_results = fifo_engine.get_results();

    assert(!fifo_results.empty());
    assert(fifo_results[0].timestamp() == 1000000030);
    assert(fifo_results[0].symbol() == "AAPL");
    assert(std::abs(fifo_results[0].pnl() - 22.50) < 0.01);

    auto lifo_engine = engine::create_engine<enums::AccountingType::LIFO>();
    lifo_engine.process_trades(fifo_result.value());
    const auto& lifo_results = lifo_engine.get_results();

    assert(!lifo_results.empty());
    assert(lifo_results[0].timestamp() == 1000000030);
    assert(lifo_results[0].symbol() == "AAPL");

    std::cout << "  ✓ File parsing tests passed" << std::endl;
    std::cout << "  FIFO generated " << fifo_results.size() << " PnL results" << std::endl;
    std::cout << "  LIFO generated " << lifo_results.size() << " PnL results" << std::endl;
}

void test_fifo_vs_lifo_difference()
{
    std::cout << "Testing FIFO vs LIFO Difference..." << std::endl;

    std::vector<types::Trade> trades = {
        types::Trade{1000000000, "AAPL", 150.00, 100, enums::TradeSide::BUY},
        types::Trade{1000000001, "AAPL", 151.00, 100, enums::TradeSide::BUY},
        types::Trade{1000000002, "AAPL", 152.00, 100, enums::TradeSide::SELL}
    };

    auto fifo_engine = engine::create_engine<enums::AccountingType::FIFO>();
    fifo_engine.process_trades(trades);
    const auto& fifo_results = fifo_engine.get_results();
    assert(fifo_results.size() == 1);
    assert(std::abs(fifo_results[0].pnl() - 200.0) < 0.01);

    auto lifo_engine = engine::create_engine<enums::AccountingType::LIFO>();
    lifo_engine.process_trades(trades);
    const auto& lifo_results = lifo_engine.get_results();
    assert(lifo_results.size() == 1);
    assert(std::abs(lifo_results[0].pnl() - 100.0) < 0.01);

    std::cout << "  ✓ FIFO vs LIFO difference tests passed" << std::endl;
}

int main()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "    PnL Calculator Test Suite" << std::endl;
    std::cout << "========================================\n" << std::endl;

    try
    {
        test_types();
        test_parser();
        test_engine_basic();
        test_partial_fills();
        test_multiple_symbols();
        test_short_selling();
        test_fifo_vs_lifo_difference();
        test_with_file();

        std::cout << "\n========================================" << std::endl;
        std::cout << "    All Tests PASSED ✓" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nTest FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}