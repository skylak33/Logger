cmake_minimum_required(VERSION 3.16)
project(Logger CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка библиотеки
option(BUILD_SHARED_LIBS ON)

file(GLOB logger_sources
    "src/*.cpp"
)

add_library(logger ${logger_sources})

target_include_directories(logger PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

add_executable(loggerApp src/main.cpp)

target_link_libraries(loggerApp PRIVATE logger)