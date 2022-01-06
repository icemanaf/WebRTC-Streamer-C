cmake_minimum_required(VERSION 3.7)
project(offerer C)

set(CMAKE_C_STANDARD 11)


set(LIBDATACHANNEL_PATH /work/libdatachannel)
set(LIBDATACHANNEL ${LIBDATACHANNEL_PATH}/build/libdatachannel.so)
include_directories(${LIBDATACHANNEL_PATH}/include)


message(STATUS "include dir: ${LIBDATACHANNEL}")

add_executable(datachannel-copy-paste-capi-offerer offerer.c)
add_executable(datachannel-copy-paste-capi-answerer answerer.c)


set_target_properties(datachannel-copy-paste-capi-offerer PROPERTIES
	OUTPUT_NAME offerer)
set_target_properties(datachannel-copy-paste-capi-answerer PROPERTIES
	OUTPUT_NAME answerer)


find_package(Threads REQUIRED)

target_link_libraries(datachannel-copy-paste-capi-answerer ${LIBDATACHANNEL} Threads::Threads)
target_link_libraries(datachannel-copy-paste-capi-offerer ${LIBDATACHANNEL} Threads::Threads)










