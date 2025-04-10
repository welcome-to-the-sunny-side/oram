cmake_minimum_required(VERSION 3.10)
project(MyBoostProject)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(/opt/homebrew/lib)

add_executable(main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES})