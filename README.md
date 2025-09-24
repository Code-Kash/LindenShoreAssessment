# PnL Calculator - Assessment

A C++20 application for calculating Profit and Loss (PnL) from trading data using FIFO or LIFO accounting methods. 

Developed on a Runpod VPS (similar to AWS EC2) running Ubuntu 22.04, gcc 12.3 (C++23 requires gcc 13+) in VSCode + VIM. 

## Compilation

```bash
make all
```

To clean and rebuild:
```bash
make clean && make all
```

## Usage

```bash
./pnl_calculator <input_file> <accounting_method>
```

Parameters:
- `input_file`: Path to CSV file containing trades
- `accounting_method`: Either `fifo` or `lifo`

## Input Format

CSV file with the following columns:
- `timestamp` - Unix timestamp
- `symbol` - Stock symbol
- `side` - B (buy) or S (sell)
- `price` - Trade price
- `quantity` - Number of shares

Example:
```csv
1000000000,AAPL,B,150.50,100
1000000010,AAPL,S,151.00,50
```

## Output Format

CSV output with PnL results:
```csv
timestamp,symbol,pnl
1000000010,AAPL,25.00
```

## Running Tests

Compile and run the test suite:
```bash
g++ -std=c++20 -Wall -Wextra -O3 -o test_runner test_main.cpp
./test_runner
```

NOTE: CMakeLists.txt is included however is unused due to building being done through the Makefile. 