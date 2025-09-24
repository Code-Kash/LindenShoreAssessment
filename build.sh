#!/bin/bash

# PnL Calculator Build Script
# Supports building with GCC 10+ and C++20

set -e  

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' 

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_prerequisites() {
    print_status "Checking prerequisites..."

    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed. Please install CMake 3.16 or later."
        exit 1
    fi

    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d'.' -f1)
    CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d'.' -f2)

    if [ "$CMAKE_MAJOR" -lt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -lt 16 ]); then
        print_error "CMake version $CMAKE_VERSION is too old. Please install CMake 3.16 or later."
        exit 1
    fi

    if ! command -v g++ &> /dev/null; then
        print_error "g++ is not installed. Please install GCC 10 or later."
        exit 1
    fi

    GCC_VERSION=$(g++ --version | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -1)
    GCC_MAJOR=$(echo $GCC_VERSION | cut -d'.' -f1)

    if [ "$GCC_MAJOR" -lt 10 ]; then
        print_error "GCC version $GCC_VERSION does not support C++20. Please install GCC 10 or later."
        exit 1
    fi

    print_success "Prerequisites check passed"
    print_status "CMake version: $CMAKE_VERSION"
    print_status "GCC version: $GCC_VERSION"
}

BUILD_TYPE="Release"
BUILD_TESTS="ON"
CLEAN_BUILD=false
INSTALL_DEPS=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --relwithdebinfo)
            BUILD_TYPE="RelWithDebInfo"
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug              Build in Debug mode"
            echo "  --release            Build in Release mode (default)"
            echo "  --relwithdebinfo     Build in RelWithDebInfo mode"
            echo "  --no-tests           Skip building tests"
            echo "  --clean              Clean build directory before building"
            echo "  --install-deps       Install Google Test automatically"
            echo "  --verbose            Enable verbose build output"
            echo "  --help               Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

if [ "$INSTALL_DEPS" = true ]; then
    print_status "Installing Google Test..."

    if command -v apt-get &> /dev/null; then
        sudo apt-get update
        sudo apt-get install -y libgtest-dev
    elif command -v yum &> /dev/null; then
        sudo yum install -y gtest-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S gtest
    else
        print_warning "Could not automatically install Google Test. CMake will download it."
    fi
fi

main() {
    print_status "Starting PnL Calculator build..."
    print_status "Build type: $BUILD_TYPE"
    print_status "Build tests: $BUILD_TESTS"

    check_prerequisites

    # Create build directory
    if [ "$CLEAN_BUILD" = true ] && [ -d "build" ]; then
        print_status "Cleaning previous build..."
        rm -rf build
    fi

    mkdir -p build
    cd build

    # Configure with CMake
    print_status "Configuring with CMake..."
    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DBUILD_TESTS="$BUILD_TESTS"
        -DCMAKE_CXX_COMPILER=g++
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )

    if [ "$VERBOSE" = true ]; then
        CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    fi

    cmake "${CMAKE_ARGS[@]}" ..

    # Build
    print_status "Building..."
    MAKE_ARGS=()

    if [ "$VERBOSE" = true ]; then
        MAKE_ARGS+=(VERBOSE=1)
    fi

    # Use all available cores for faster compilation
    CORES=$(nproc 2>/dev/null || echo 1)
    make -j"$CORES" "${MAKE_ARGS[@]}"

    print_success "Build completed successfully!"

    # Run tests if built
    if [ "$BUILD_TESTS" = "ON" ]; then
        print_status "Running tests..."
        if ctest --output-on-failure; then
            print_success "All tests passed!"
        else
            print_error "Some tests failed!"
            exit 1
        fi
    fi

    # Print build artifacts
    print_status "Build artifacts:"
    echo "  Executable: $(pwd)/pnl_calculator"
    if [ "$BUILD_TESTS" = "ON" ]; then
        echo "  Test executable: $(pwd)/pnl_calculator_tests"
    fi
    echo "  Compile commands: $(pwd)/compile_commands.json"

    print_success "Build process completed successfully!"
}

main