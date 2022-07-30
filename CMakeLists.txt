cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set (CMAKE_CXX_FLAGS "-O3 -pthread -DNDEBUG -lssl -lcrypto")
set(CMAKE_BUILD_TYPE "Release")
project(hftbot)
  
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(Boost 1.79.0 REQUIRED COMPONENTS system thread filesystem container)
find_package(Threads REQUIRED)

set(URLIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/url/include")
set(URLIB_HEADERS ${URLIB_DIRECTORY}/boost/url.hpp ${URLIB_DIRECTORY}/boost/url/src.hpp)
set(SOURCES core/features.cpp adapters/binance/binance-ws.cpp src/main.cpp adapters/binance/binance-http.cpp adapters/utils/utils.cpp adapters/ftx/ftx-http.cpp adapters/ftx/ftx-ws.cpp adapters/coinbase/coinbase-http.cpp adapters/coinbase/coinbase-ws.cpp adapters/kraken/kraken-http.cpp adapters/kraken/kraken-ws.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${URLIB_DIRECTORY})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/adapters/binance ${PROJECT_SOURCE_DIR}/core ${PROJECT_SOURCE_DIR}/adapters/ftx ${PROJECT_SOURCE_DIR}/adapters/coinbase ${PROJECT_SOURCE_DIR}/adapters/kraken ${PROJECT_SOURCE_DIR}/adapters/utils)

target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

