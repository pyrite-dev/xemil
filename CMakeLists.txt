cmake_minimum_required(VERSION 3.25)
project(
	xemil
)

include(CheckIncludeFiles)

include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(
	GLOB
	SOURCES
	src/*.c
)

add_library(
	xemil
	SHARED
	${SOURCES}
)

target_include_directories(
	xemil
	PUBLIC
	include
)

install(
	TARGETS xemil
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
	DIRECTORY include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h"
)
