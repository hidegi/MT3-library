set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)
add_executable(mt3_simple_test mt3_simple_test.cpp)
target_link_libraries(mt3_simple_test 
	PRIVATE 
	GTest::GTest
	mt3)
add_test(mt3_simple_test mt3_simple_test)