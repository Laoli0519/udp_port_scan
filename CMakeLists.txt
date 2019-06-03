cmake_minimum_required(VERSION 2.8)
project(udp_port_scan)

if (CMAKE_VERSION VERSION_LESS "3.1")
	add_definitions(-std=c99)
else ()
	set(CMAKE_C_FLAGS "-std=c99")
endif ()

set(CMAKE_VERBOSE_MAKEFILE yes)

if(APPLE)
	set(CMAKE_MACOSX_RPATH ON)
	set(CMAKE_SKIP_BUILD_RPATH TRUE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()


set(UDP_SCAN_PORT_SOURCES
		src/udpPortScan.c
		src/udpPortScan.c
		src/optparse.h
)


include_directories(src)
add_library(core_static STATIC ${UDP_SCAN_PORT_SOURCES})
link_libraries(core_static)

#target_link_libraries(udpScanPorts Udp)


add_executable(udp_scan_port app/main.c)