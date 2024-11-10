link_libraries(mot)
link_directories(${CMAKE_BINARY_DIR}/src)

add_executable(sp_serialization_test sp_serialization_test.cpp)
add_executable(sp_tree_test sp_tree_test.cpp rbt_ops.cpp)