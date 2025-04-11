set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)
project(BoostMultiTarget)

set(CMAKE_CXX_STANDARD 17)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# Include your headers if needed
include_directories(${CMAKE_SOURCE_DIR})

# === Client Executable ===
add_executable(client
    src/client/client_main.cpp
    src/client/client_stash.cpp
    src/oram/block.cpp
    src/oram/bucket.cpp
)

target_link_libraries(client PRIVATE Boost::system)

# === Server Executable ===
add_executable(server
    src/server/server_main.cpp
    src/oram/block.cpp
    src/oram/bucket.cpp
)

target_link_libraries(server PRIVATE Boost::system)