cmake_minimum_required(VERSION 3.16)

project(baguatool CXX C)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(FATAL_ERROR "Do not support non-Linux now!")
endif()


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

# explicitly list all source file
add_library(
  baguatool
  SHARED
  
  src/common/utils.cpp
  src/core/pag.cpp

  src/GraphPerf/preprocessing/preprocess.cpp
  src/GraphSD/dyninst/static_analysis.cpp
)


set_property(TARGET baguatool PROPERTY CXX_STANDARD 17)


target_compile_options(
  baguatool PRIVATE
  -Wall -Wno-missing-braces
  #-fno-rtti # We cannot open it because dyninst use `typeid` in header
  -fno-omit-frame-pointer -fstack-protector-all
  $<$<CONFIG:DEBUG>:-fsanitize=address -static-libasan>)

target_include_directories(baguatool PUBLIC ${PROJECT_SOURCE_DIR}/include)
# TODO: the following directory should be PRIVATE in the future
target_include_directories(baguatool PUBLIC src)
target_include_directories(baguatool PRIVATE ${PROJECT_SOURCE_DIR}/third_party/dbg)
target_include_directories(baguatool PRIVATE igraph)#${PROJECT_BINARY_DIR}/third_party/igraph/include)# ${DYNINST_LIBRARIES} ${PAPI_LIBRARIES})
target_include_directories(baguatool PRIVATE ${DYNINST_INCLUDE_DIR})
target_include_directories(baguatool PRIVATE ${PAPI_INCLUDE_DIRS})

target_link_libraries(baguatool PRIVATE igraph)
target_link_libraries(
  baguatool
  PRIVATE
  # TODO: we do not need all these
  dyninstAPI parseAPI instructionAPI symtabAPI symLite dynDwarf dynElf boost_system common elf)
target_link_libraries(baguatool PRIVATE ${PAPI_LIBRARIES})


if (ENABLE_EXAMPLE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/example/simple)
endif()

if (ENABLE_TEST)
  # List what we want to test
  add_subdirectory(${PROJECT_SOURCE_DIR}/test/igraph_test)
endif()