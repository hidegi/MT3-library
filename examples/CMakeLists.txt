add_executable(json2bto json2bto/main.cpp)
add_executable(printbto printbto/main.cpp)
target_link_libraries(json2bto mt3)
target_link_libraries(printbto mt3)
