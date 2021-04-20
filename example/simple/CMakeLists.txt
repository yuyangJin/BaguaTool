#add_executable(preprocess preprocess.cpp)
add_executable(static_analysis_example static_analysis_example.cpp)
add_executable(read_perf_data read_perf_data.cpp)
add_library(single_process_sampler SHARED single_process_sampler.cpp)

#target_link_libraries(preprocess PRIVATE baguatool)
target_link_libraries(static_analysis_example PRIVATE baguatool)
target_link_libraries(single_process_sampler PRIVATE baguatool)
target_link_libraries(read_perf_data PRIVATE baguatool)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_CURRENT_BINARY_DIR}/run.sh COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cg.A.4 ${CMAKE_CURRENT_BINARY_DIR}/cg.A.4 COPYONLY)