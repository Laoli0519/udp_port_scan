
include_directories(${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(. DIR_SRCS)

add_executable(udpPortScans ${DIR_SRCS})

target_link_libraries(udpPortScans libPortScan)


