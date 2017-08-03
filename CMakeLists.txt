# Copyright (C) 2017  olibre <olibre@lmap.org>
# Copyright 2017 Garrett D'Amore <garrett@damore.org>
#
# You may distribute under the terms of either the GNU General Public
# License or the Less License, as specified in the README file.
#
# For more information, see the README file.

cmake_minimum_required(VERSION 3.1)
include(CheckFunctionExists)

project(less-fork VERSION 1.0.1 LANGUAGES C)

set(SYSNAME	"gdamore")
set(RELEASE ${PROJECT_VERSION})
set(PATCHID "unbundled")

# prefer c99 if possible - this may be necessary to enable latest
# versions of POSIX standards (SUSv3 requires it).
set(CMAKE_C_STANDARD 99)

# disable GNU extensions etc.
set(CMAKE_C_EXTENSIONS OFF)

# only for version.c
add_definitions(-D SYSNAME='"${SYSNAME}"'
                -D RELEASE='"${RELEASE}"'
                -D VERSION='"${PATCHID}"')

# pick up 64-bit file size support
add_definitions(-D _FILE_OFFSET_BITS=64)

find_package(Curses REQUIRED)

# for less and lesskey
include_directories(.)

set(LESSKEYFILE_SYS "${CMAKE_INSTALL_PREFIX}/etc/sysless" CACHE FILEPATH "System wide lesskey file.")
add_definitions(-D LESSKEYFILE_SYS='"${LESSKEYFILE_SYS}"')

check_function_exists(strlcat have_strlcat)
check_function_exists(strlcpy have_strlcpy)
check_function_exists(vasprintf have_vasprintf)

if (have_strlcat)
	add_definitions(-D HAVE_STRLCAT=1)
else()
	set(compat_sources ${compat_sources} strlcat.c)
endif()

if (have_strlcpy)
	add_definitions(-D HAVE_STRLCPY=1)
else()
	set(compat_sources ${compat_sources} strlcpy.c)
endif()

if (have_vasprintf)
	add_definitions(-D HAVE_VASPRINTF=1)
endif()

add_executable(mkhelp mkhelp.c)
add_custom_command(OUTPUT     help.c COMMAND mkhelp < ${CMAKE_SOURCE_DIR}/less.hlp > help.c)
add_custom_command(OUTPUT morehelp.c COMMAND mkhelp < ${CMAKE_SOURCE_DIR}/more.hlp | sed -e s/helpdata/morehelpdata/g > morehelp.c)
add_custom_target(generate_help_files DEPENDS help.c morehelp.c)
add_dependencies (generate_help_files mkhelp) 

set(lesskey_headers lesskey.h less.h cmd.h funcs.h defines.h)

set(less_headers ${lesskey_headers} charset.h option.h
                 pattern.h position.h)

source_group("less headers" FILES ${less_headers})
add_executable(less
        main.c screen.c brac.c ch.c charset.c cmdbuf.c 
        command.c cvt.c decode.c edit.c filename.c forwback.c 
        help.c ifile.c input.c jump.c line.c linenum.c 
        lsystem.c mark.c optfunc.c option.c opttbl.c os.c
        output.c pattern.c position.c prompt.c search.c
        signal.c tags.c ttyin.c morehelp.c version.c
        ${less_headers} ${compat_sources})
target_include_directories(less SYSTEM PUBLIC ${CURSES_INCLUDE_DIR})
target_link_libraries(less ${CURSES_LIBRARIES})
add_dependencies(less generate_help_files) 

source_group("lesskey headers" FILES ${lesskey_headers})
add_executable(lesskey
               lesskey.c version.c 
               ${lesskey_headers})
