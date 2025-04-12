set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)
project(BoostMultiTarget)

set(CMAKE_CXX_STANDARD 17)

# Find libsodium
set(SODIUM_ROOT /opt/homebrew)
include_directories(${SODIUM_ROOT}/include)
link_directories(${SODIUM_ROOT}/lib)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system serialization)
include_directories(${Boost_INCLUDE_DIRS})

# Set OpenSSL root directory and additional paths
set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib")

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found")
endif()

# Use modern CMake target-based approach
include_directories(${OPENSSL_INCLUDE_DIR})

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Include your headers if needed
include_directories(${CMAKE_SOURCE_DIR})

# === Client Executable ===
add_executable(client
    src/client/client_test.cpp
)

target_include_directories(client PRIVATE 
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(client PRIVATE 
    Boost::system 
    sodium
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# === Server Executable ===
add_executable(server
    src/server/server_main.cpp
    # src/oram/block.cpp
    # src/oram/bucket.cpp
)

target_include_directories(server PRIVATE 
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(server PRIVATE 
    Boost::system 
    Boost::serialization
    sodium
    OpenSSL::SSL 
    OpenSSL::Crypto
)