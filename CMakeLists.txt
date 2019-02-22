cmake_minimum_required(VERSION 2.8)
project(udp_port_scan)

if (CMAKE_VERSION VERSION_LESS "3.1")
	add_definitions(-std=c99)
else ()
	set(CMAKE_C_FLAGS "-std=c99")
endif ()

#aux_source_directory(. DIR_SRCS)

#include_directories(src)

#add_executable(udpScanPorts ${DIR_SRCS})
#add_executable(udp_scan_port ${DIR_SRCS})

add_subdirectory(src)
add_subdirectory(lib)

#target_link_libraries(udpScanPorts Udp)


