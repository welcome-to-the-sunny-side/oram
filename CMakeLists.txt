set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)
project(ORAM)

set(CMAKE_CXX_STANDARD 17)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system serialization)
include_directories(${Boost_INCLUDE_DIRS})

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found")
endif()

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Include your headers if needed
include_directories(${CMAKE_SOURCE_DIR})

# Create ORAM library interface
add_library(oram_lib INTERFACE)
target_include_directories(oram_lib INTERFACE 
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(oram_lib INTERFACE
    Boost::system 
    Boost::serialization
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# === Client Executables ===
add_executable(client_test
    src/client/client_test.cpp
)

target_link_libraries(client_test PRIVATE oram_lib)


# === Server Executable ===
add_executable(server
    src/server/server_main.cpp
)

target_link_libraries(server PRIVATE oram_lib)

# === Benchmark Executable ===

add_executable(client_benchmark
    benchmark/client_benchmark.cpp
)

target_link_libraries(client_benchmark PRIVATE oram_lib)

add_executable(oram_benchmark
    benchmark/oram_benchmark.cpp
)

target_link_libraries(oram_benchmark PRIVATE oram_lib)
