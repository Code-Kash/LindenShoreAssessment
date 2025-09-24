CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -I./include
LDFLAGS =

SOURCES = src/main.cpp src/pnl_calculator_types.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = pnl_calculator

TEST_SOURCES = tests/test_main.cpp tests/test_types.cpp tests/test_parser.cpp tests/test_engine.cpp tests/test_integration.cpp src/pnl_calculator_types.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TEST_TARGET = pnl_calculator_tests

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS) -lgtest -lgtest_main -pthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)

# Demo with test data
demo: $(TARGET)
	@echo "Running FIFO demo:"
	./$(TARGET) test_data.csv fifo
	@echo ""
	@echo "Running LIFO demo:"
	./$(TARGET) test_data.csv lifo

install:
	@echo "For a complete build system with dependency management, use:"
	@echo "  ./build.sh  (requires CMake 3.16+ and GCC 10+)"
	@echo ""
	@echo "For manual build:"
	@echo "  make        (basic build with current compiler)"
	@echo "  make demo   (run demo with test data)"
	@echo "  make test   (build and run tests - requires gtest)"

.DEFAULT_GOAL := install