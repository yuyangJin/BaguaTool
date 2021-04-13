cmake_minimum_required(VERSION 3.16)

project(baguatool CXX C)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(FATAL_ERROR "Do not support non-Linux now!")
endif()

# TODO: remove it in the future
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Set build type to DEBUG" FORCE)
message(AUTHOR_WARNING "Overwrite build type to DEBUG for development")

# third party
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/igraph)
target_compile_options(igraph PRIVATE -fPIC)

find_package(PAPI REQUIRED)
find_package(Dyninst REQUIRED)
if (Dyninst_FOUND) 
  message(STATUS "Found Dyninst: " ${DYNINST_INCLUDE_DIR})
endif()

option(ENABLE_TEST "Enable test" ON)
option(ENABLE_EXAMPLE "Enable example" ON)
option(ENABLE_SANITIZE "Enable sanitize" OFF)

# explicitly list all source file
add_library(
  baguatool
  SHARED
  
  src/common/utils.cpp
  src/core/graph.cpp
  src/core/pag.cpp
  src/core/perf_data.cpp

  #src/GraphPerf/preprocessing/preprocess.cpp
  src/GraphSD/dyninst/static_analysis.cpp
  src/GraphSD/papi/sampler.cpp
)

set_property(TARGET baguatool PROPERTY CXX_STANDARD 17)
set_property(TARGET baguatool PROPERTY CXX_STANDARD_REQUIRED ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0.0")
    message(FATAL_ERROR "Insufficient GCC version (${CMAKE_CXX_COMPILER_VERSION}), need > 7.0.0")
  endif()
else()
  message(FATAL_ERROR "Do not support cxx compiler for ${CMAKE_CXX_COMPILER_ID}, choose GNU please")
endif()

target_compile_options(
  baguatool PRIVATE
  -Wall -Wno-missing-braces
  #-fno-rtti # We cannot open it because dyninst use `typeid` in header
  -fno-omit-frame-pointer -fstack-protector-all
)

# FIXME: how to use sanitize for subproject?
if (ENABLE_SANITIZE) 
  target_compile_options(
    baguatool PUBLIC
    $<$<CONFIG:DEBUG>:-fsanitize=address -g>
  )
endif()

target_include_directories(baguatool PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(baguatool PRIVATE src)
target_include_directories(baguatool PRIVATE ${PROJECT_SOURCE_DIR}/third_party/dbg)
target_include_directories(baguatool PRIVATE igraph)
target_include_directories(baguatool PRIVATE ${DYNINST_INCLUDE_DIR})
target_include_directories(baguatool PRIVATE ${PAPI_INCLUDE_DIRS})

target_link_libraries(baguatool PRIVATE igraph)
target_link_libraries(
  baguatool
  PRIVATE
  # TODO: we do not need all these
  dyninstAPI parseAPI instructionAPI symtabAPI symLite dynDwarf dynElf boost_system common elf unwind papi)
target_link_libraries(baguatool PRIVATE ${PAPI_LIBRARIES})


if (ENABLE_EXAMPLE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/example/simple)
endif()

if (ENABLE_TEST)
  # List what we want to test
  add_subdirectory(${PROJECT_SOURCE_DIR}/test/igraph_test)
endif()