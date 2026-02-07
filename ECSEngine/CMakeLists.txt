cmake_minimum_required(VERSION 3.10)
project(ECSEngine CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)

# Set the source directory
set(PROJECT_SOURCES
    src/PrivimiteTypes.h
    src/ECS/Common.h
    src/ECS/ECS.h
    src/ECS/Component.h
    src/ECS/Component.cpp
    src/ECS/Entity.h
    src/ECS/Entity.inl
    src/ECS/IComponent.h
    src/ECS/IPool.h
    src/ECS/Pool.h
    src/ECS/Registry.cpp
    src/ECS/Registry.h
    src/ECS/System.cpp
    src/ECS/System.h
)

# Add the executable and include all source files
add_library(ECSEngine STATIC ${PROJECT_SOURCES})

# Add include directories
target_include_directories(ECSEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enable testing functionality
enable_testing()

# Find GoogleTest
find_package(GTest CONFIG REQUIRED)

# Add test source files
set(TEST_SOURCES
    tests/ecs_gtest.cpp
)

# Create test executable
add_executable(ECSEngineTests ${TEST_SOURCES})

# Link test executable with gtest and your engine
target_link_libraries(ECSEngineTests
    PRIVATE
    ECSEngine
    GTest::gtest
    GTest::gtest_main
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(ECSEngineTests)

