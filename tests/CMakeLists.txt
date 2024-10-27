link_libraries(playground)
link_directories(${CMAKE_BINARY_DIR}/src)

add_executable(sp_serialization_test sp_serialization_test.cpp)
