cmake_minimum_required(VERSION 3.16)
project(fitness_club_tracker C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sqlite3 STATIC third_party/sqlite/sqlite3.c)
target_compile_definitions(sqlite3 PRIVATE SQLITE_OMIT_LOAD_EXTENSION)
target_include_directories(sqlite3 PUBLIC third_party/sqlite)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(fitness_club ${APP_SOURCES})
target_include_directories(fitness_club PRIVATE src)
target_link_libraries(fitness_club PRIVATE sqlite3)
if(UNIX)
  target_link_libraries(fitness_club PRIVATE pthread dl)
endif()
