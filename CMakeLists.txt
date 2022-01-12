cmake_minimum_required(VERSION 3.7)
project(WebRTCStreamer)

set(CMAKE_C_STANDARD 11)



find_package(PkgConfig)
find_package(Threads REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4 gstreamer-base-1.0>=1.4)

set(LIBDATACHANNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../libdatachannel)
set(LIBDATACHANNEL ${LIBDATACHANNEL_PATH}/build/libdatachannel.so)
include_directories(${LIBDATACHANNEL_PATH}/include ${GST_INCLUDE_DIRS})


add_executable(WebRTCStreamer main.c)


set_target_properties(WebRTCStreamer PROPERTIES
	OUTPUT_NAME WebRTCStreamer)

target_link_libraries(WebRTCStreamer glib-2.0  ${LIBDATACHANNEL} Threads::Threads  ${GST_LIBRARIES})











