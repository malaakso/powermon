cmake_minimum_required(VERSION 2.8.12)
project (powermon)

# Set the default install path to /usr
if (NOT WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "default install path" FORCE)
endif()

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/sbin"
	CACHE PATH "Installation directory for binaries")

add_executable(powermon Adc.cpp InfluxdbWriter.cpp powermon.cpp)

set (CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
target_link_libraries(powermon iio curl)

install(TARGETS powermon RUNTIME DESTINATION "${INSTALL_BIN_DIR}")
