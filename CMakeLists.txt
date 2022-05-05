cmake_minimum_required(VERSION 3.18.0)

project(hftbot)
  
find_package(Boost 1.79.0 REQUIRED COMPONENTS system thread filesystem container)
find_package(Threads REQUIRED)

# add_subdirectory(external/urlib)

set(URLIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/urlib/include")
set(URLIB_HEADERS ${URLIB_DIRECTORY}/boost/url.hpp ${URLIB_DIRECTORY}/boost/url/src.hpp)
set(SOURCES src/httpClient.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${URLIB_HEADERS})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${URLIB_DIRECTORY})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC ${JSONCPP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto)

