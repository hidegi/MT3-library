set(LIBRARY_PREFIX SP)
list(APPEND sources
	${LIBRARY_PREFIX}/test/unit.c
	${LIBRARY_PREFIX}/sparse/mot.c
	${LIBRARY_PREFIX}/sparse/buffer.c
	${LIBRARY_PREFIX}/utils/io.cpp
	${LIBRARY_PREFIX}/cnc/thread.cpp
	${LIBRARY_PREFIX}/cnc/thread_impl.cpp
)
if(UNIX)
list(APPEND sources ${LIBRARY_PREFIX}/cnc/platform/thread_impl_posix.cpp)
else()
list(APPEND sources ${LIBRARY_PREFIX}/cnc/platform/thread_impl_win32.cpp)
endif()
add_library(mot ${sources})
