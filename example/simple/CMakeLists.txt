add_executable(preprocess preprocess.cpp)
add_executable(static_analysis static_analysis.cpp)

target_link_libraries(preprocess PRIVATE baguatool)
target_link_libraries(static_analysis PRIVATE baguatool)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_CURRENT_BINARY_DIR}/run.sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cg.A.4 ${CMAKE_CURRENT_BINARY_DIR}/cg.A.4 COPYONLY)