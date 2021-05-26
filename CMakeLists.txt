cmake_minimum_required (VERSION 3.8)
project (Ising)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
endif()
add_executable (Ising "Ising.cpp")

