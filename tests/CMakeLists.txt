#set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
#include(FetchContent)
#FetchContent_Declare(
#	googletest
#	GIT_REPOSITORY https://github.com/google/googletest.git
#	GIT_TAG release-1.11.0
#)
#FetchContent_MakeAvailable(googletest)
#add_library(GTest::GTest INTERFACE IMPORTED)
#target_link_libraries(GTest::GTest INTERFACE gtest_main)
#add_executable(sp_test_mt3_api sp_test_mt3_api.cpp)
#target_link_libraries(sp_test_mt3_api 
#	PRIVATE 
#	GTest::GTest
#	mt3)
#add_test(sp_test_mt3_api sp_test_mt3_api)
#
#add_executable(sp_serialization_test sp_serialization_test.cpp)
#add_executable(sp_bto_test sp_bto_test.cpp)
#add_executable(sp_tree_test sp_tree_test.cpp rbt_ops.cpp)
#add_executable(sp_mt3_linked_roots_test sp_mt3_linked_roots_test.cpp)
##add_test(NAME sp_test_mt3_api COMMAND sp_test_mt3_api)
#
#target_link_libraries(sp_serialization_test mt3)
#target_link_libraries(sp_tree_test mt3)
#target_link_libraries(sp_bto_test mt3)
#target_link_libraries(sp_mt3_linked_roots_test mt3)

add_executable(simpletest mt3_serialization_test.cpp)
target_link_libraries(simpletest mt3)