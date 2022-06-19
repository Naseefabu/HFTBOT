cmake_minimum_required(VERSION 3.18.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O2 -pthread -lssl -lcrypto")
set(CMAKE_BUILD_TYPE "Release")
project(hftbot)
  
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(Boost 1.79.0 REQUIRED COMPONENTS system thread filesystem container)
find_package(Threads REQUIRED)

set(URLIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/url/include")
set(URLIB_HEADERS ${URLIB_DIRECTORY}/boost/url.hpp ${URLIB_DIRECTORY}/boost/url/src.hpp)
set(SOURCES exchanges/binance-ws.cpp src/main.cpp exchanges/binance-http.cpp src/utils.cpp exchanges/ftx-http.cpp exchanges/ftx-ws.cpp exchanges/coinbase-http.cpp exchanges/coinbase-ws.cpp exchanges/kraken-http.cpp exchanges/kraken-ws.cpp src/database.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${URLIB_DIRECTORY})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/exchanges)

target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

