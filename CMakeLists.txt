cmake_minimum_required(VERSION 3.18.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O3 -pthread -lssl -lcrypto")
set(CMAKE_BUILD_TYPE "Release")
project(hftbot)
  
find_package(nlohmann_json 3.7.0 REQUIRED)
find_package(Boost 1.79.0 REQUIRED COMPONENTS system thread filesystem container)
find_package(Threads REQUIRED)

# add_subdirectory(external/urlib)

set(URLIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/url/include")
set(URLIB_HEADERS ${URLIB_DIRECTORY}/boost/url.hpp ${URLIB_DIRECTORY}/boost/url/src.hpp)
set(SOURCES src/binance-ws.cpp src/main.cpp src/boosturl.cpp src/binance-http.cpp src/utils.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${URLIB_DIRECTORY})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC ${JSONCPP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

