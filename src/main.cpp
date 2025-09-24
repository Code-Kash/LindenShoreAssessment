#include "../include/pnl_calculator_engine.h"
#include "../include/pnl_calculator_parser.h"
#include "../include/pnl_calculator_types.h"
#include "../include/pnl_calculator_constants.h"
#include "../include/pnl_calculator_enums.h"
#include <iostream>
#include <string>
#include <variant>

namespace pnl::app
{
    void print_usage(const char* program_name)
    {
        std::cerr << "Usage: " << program_name << " <input_file> <accounting_method>\n"
                  << "  input_file: Path to CSV file containing trades\n"
                  << "  accounting_method: 'fifo' or 'lifo'\n"
                  << "\nExample:\n"
                  << "  " << program_name << " trades.csv fifo\n";
    }

    template<enums::AccountingType Method>
    int run_calculation(const std::string& filename)
    {
        auto trades_result = parser::CSVParser::parse_file(filename);
        if (!trades_result) [[unlikely]]
        {
            std::cerr << "Error parsing file: Could not open file: " << filename << std::endl;
            return constants::ERROR_PARSE_ERROR;
        }

        const auto& trades = trades_result.value();
        if (trades.empty()) [[unlikely]]
        {
            std::cerr << "Warning: No trades found in file" << std::endl;
            std::cout << constants::CSV_HEADER << std::endl;
            return constants::SUCCESS;
        }

        auto engine = engine::create_engine<Method>();
        engine.process_trades(trades);

        const auto& results = engine.get_results();

        std::cout << constants::CSV_HEADER << std::endl;

        for (const auto& result : results)
        {
            std::cout << result.to_csv_string() << std::endl;
        }

        return constants::SUCCESS;
    }

    int process_with_accounting_method(const std::string& filename, enums::AccountingType method)
    {
        switch (method)
        {
            case enums::AccountingType::FIFO:
                return run_calculation<enums::AccountingType::FIFO>(filename);
            case enums::AccountingType::LIFO:
                return run_calculation<enums::AccountingType::LIFO>(filename);
        }
        return run_calculation<enums::AccountingType::FIFO>(filename);
    }
}

int main(int argc, char* argv[])
{
    using namespace pnl;

    if (argc != 3) [[unlikely]]
    {
        app::print_usage(argv[0]);
        return constants::ERROR_INVALID_ARGS;
    }

    const std::string filename = argv[1];
    const std::string accounting_method = argv[2];

    if (accounting_method != constants::FIFO_ARG && accounting_method != constants::LIFO_ARG) [[unlikely]]
    {
        std::cerr << "Error: Invalid accounting method '" << accounting_method
                  << "'. Must be 'fifo' or 'lifo'." << std::endl;
        app::print_usage(argv[0]);
        return constants::ERROR_INVALID_ACCOUNTING;
    }

    const auto method = utils::string_to_accounting_type(accounting_method);

    try
    {
        return app::process_with_accounting_method(filename, method);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return constants::ERROR_PARSE_ERROR;
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred" << std::endl;
        return constants::ERROR_PARSE_ERROR;
    }
}