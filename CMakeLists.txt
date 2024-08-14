cmake_minimum_required(VERSION 3.9)
project(spdkpp VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# C standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_C_STANDARD)
  # This project can use C11, but will gracefully decay down to C89.
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED OFF)
  set(CMAKE_C_EXTENSIONS OFF)
endif(NOT CMAKE_C_STANDARD)

# C++ standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_CXX_STANDARD)
  # This project requires C++11.
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

add_compile_options(-fpermissive -Og)
option(LEVELDB_BUILD_TESTS "Build LevelDB's unit tests" ON)

if(BUILD_SHARED_LIBS)
  # Only export LEVELDB_EXPORT symbols from the shared library.
  add_compile_options(-fvisibility=hidden)
endif(BUILD_SHARED_LIBS)

# Must be included before CMAKE_INSTALL_INCLUDEDIR is used.
include(GNUInstallDirs)

include(FetchContent)
include(BuildSPDK)
build_spdk()

FetchContent_Declare(
  SPDLOG
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.x
)
FetchContent_MakeAvailable(SPDLOG)
add_compile_definitions(HAS_SPDLOG)
include_directories("/home/wineway/sqk-sched-core/src/core")
add_executable(spdkpp "")
message("SPDK_INCLUDE_DIR: ${SPDK_INCLUDE_DIR}")
target_include_directories(spdkpp PUBLIC ${SPDK_INCLUDE_DIR} ${spdlog_SOURCE_DIR}/include)
target_link_libraries(spdkpp spdk::spdk spdlog)
target_sources(spdkpp
  PRIVATE
  "src/main.cpp"
)


target_include_directories(spdkpp
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


target_compile_definitions(spdkpp
  PRIVATE
)
