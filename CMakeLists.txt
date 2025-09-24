cmake_minimum_required(VERSION 3.16)

project(pnl_calculator
    VERSION 1.0.0
    DESCRIPTION "High-performance PnL Calculator with FIFO/LIFO accounting"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
        message(FATAL_ERROR "GCC version must be at least 10.0 for C++20 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(FATAL_ERROR "Clang version must be at least 12.0 for C++20 support")
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wunused
    -Wuninitialized
    -Wlogical-op
    -Wduplicated-cond
    -Wduplicated-branches
    -Wnull-dereference
    -Wdouble-promotion
    -Wshadow
    -Wformat=2
)

set(PERFORMANCE_FLAGS
    -ffast-math
    -funroll-loops
    -finline-functions
    -fomit-frame-pointer
    -flto
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/pnl_calculator_types.cpp
)

set(HEADERS
    include/pnl_calculator_accountingtraits.h
    include/pnl_calculator_concepts.h
    include/pnl_calculator_constants.h
    include/pnl_calculator_engine.h
    include/pnl_calculator_enums.h
    include/pnl_calculator_macros.h
    include/pnl_calculator_parser.h
    include/pnl_calculator_types.h
)

add_executable(pnl_calculator ${SOURCES} ${HEADERS})

target_compile_options(pnl_calculator PRIVATE ${WARNING_FLAGS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(pnl_calculator PRIVATE ${PERFORMANCE_FLAGS})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET pnl_calculator PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

find_package(Threads REQUIRED)
target_link_libraries(pnl_calculator PRIVATE Threads::Threads)

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    find_package(GTest QUIET)

    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
        )

        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    set(TEST_SOURCES
        tests/test_main.cpp
        tests/test_types.cpp
        tests/test_parser.cpp
        tests/test_engine.cpp
        tests/test_integration.cpp
        src/pnl_calculator_types.cpp
    )

    add_executable(pnl_calculator_tests ${TEST_SOURCES} ${HEADERS})

    target_compile_options(pnl_calculator_tests PRIVATE ${WARNING_FLAGS})

    if(GTest_FOUND)
        target_link_libraries(pnl_calculator_tests PRIVATE
            GTest::gtest
            GTest::gtest_main
            Threads::Threads
        )
    else()
        target_link_libraries(pnl_calculator_tests PRIVATE
            gtest
            gtest_main
            Threads::Threads
        )
    endif()

    enable_testing()

    add_test(NAME pnl_calculator_unit_tests COMMAND pnl_calculator_tests)

    add_custom_target(test_verbose
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose
        DEPENDS pnl_calculator_tests
    )
endif()

# Install targets
install(TARGETS pnl_calculator
    RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTS}")