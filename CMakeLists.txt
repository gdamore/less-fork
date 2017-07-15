# Copyright (C) 2017  olibre <olibre@lmap.org>
#
# You may distribute under the terms of either the GNU General Public
# License or the Less License, as specified in the README file.
#
# For more information, see the README file.

cmake_minimum_required(VERSION 3.1)

project(less-fork VERSION 1.0.0 LANGUAGES C)

set(SYSNAME	"gdamore")
set(RELEASE ${PROJECT_VERSION})
set(PATCHID "unbundled")

# only for version.c
add_definitions(-D SYSNAME='"${SYSNAME}"'
                -D RELEASE='"${RELEASE}"'
                -D VERSION='"${PATCHID}"')

# for less and lesskey
add_definitions(-D SYSDIR='"/etc"')
include_directories(.)

add_library(compat strlcat.c strlcpy.c)

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
        ${less_headers})
target_link_libraries(less compat curses)
add_dependencies(less generate_help_files) 

source_group("lesskey headers" FILES ${lesskey_headers})
add_executable(lesskey
               lesskey.c version.c 
               ${lesskey_headers})
