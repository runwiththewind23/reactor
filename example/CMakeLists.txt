# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(EchoServerProject)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

# Include directory
include_directories(include)

# Source files
file(GLOB CLIENT_SRC "src/bankclient.cpp")
file(GLOB ECHOSERVER_SRC "src/*.cpp")

# Add the client executable
add_executable(bankclient ${CLIENT_SRC})

# Remove client.cpp from the server build
list(REMOVE_ITEM ECHOSERVER_SRC ${CLIENT_SRC})

# Add the echoserver executable
add_executable(bankserver ${ECHOSERVER_SRC})

# Link libraries
target_link_libraries(bankserver pthread)
