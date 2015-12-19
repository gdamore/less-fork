#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright 2015 Garrett D'Amore <garrett@damore.org>
#

#
# You will probably want to select a value for STD below.  This makefile works
# with traditional Sun Make, and GNU make.  It doesn't do .KEEP_STATE,
# though.
#

PROGS=		less lesskey more mkhelp

# STD can be default, SUSv2, SUSv3, SUSv4, MACOS, or ILLUMOS
# default works for most platforms (but not illumos!)
STD =

DEBUG =		yes
OPTIMIZE =	yes

LF64 =		-D _FILE_OFFSET_BITS=64

DEFS_SUSv4 =	-D _XOPEN_SOURCE=700 $(LF64)
CC_SUSv4 =	c99
OPT_SUSv4 =
DBG_SUSv4 =

DEFS_SUSv3 =	-D _XOPEN_SOURCE=600 $(LF64)
CC_SUSv3 =	c99
OPT_SUSv3 =
DBG_SUSv3 =

DEFS_SUSv2 =	-D _XOPEN_SOURCE=500
CC_SUSv2 =	c89
OPT_SUSv2 =
DBG_SUSv2 =

DEFS_MACOS =	-D HAVE_VASPRINTF -DHAVE_STRLCPY -DHAVE_STRLCAT
CC_MACOS =	clang
OPT_MACOS =	-O
DBG_MACOS =	-g

DEFS_ILLUMOS =	$(DEFS_SUSv3) -D __EXTENSIONS__  \
		-D HAVE_VASPRINTF -D HAVE_STRLCPY -D HAVE_STRLCAT
CC_ILLUMOS =	c99 -v -errwarn=%all
OPT_ILLUMOS =	-xO2
DBG_ILLUMOS =	-g

DEFS_default =	$(LF64) -D HAVE_VASPRINTF
CC_default =	cc
OPT_default =
DBG_default =

CC =		$(CC_$(STD)) $(DEFS_$(STD)) $(OPTFLAG) $(DBGFLAG)

LINT_gcc =	$(CC) -Werror -Wall -pedantic
LINT_spro = 	lint
LINT =		$(LINT_gcc)

RM=		rm -f
LN=		ln
SED=		sed
CSTYLE=		cstyle -cPp

compat_OBJS =	strlcat.o strlcpy.o

less_OBJS =	main.o screen.o brac.o ch.o charset.o cmdbuf.o		\
		command.o cvt.o decode.o edit.o filename.o forwback.o	\
		help.o ifile.o input.o jump.o line.o linenum.o		\
		lsystem.o mark.o optfunc.o option.o opttbl.o os.o	\
		output.o pattern.o position.o prompt.o search.o		\
		signal.o tags.o ttyin.o morehelp.o version.o

less_SRCS =	$(less_OBJS:%.o=%.c)

lesskey_OBJS =	lesskey.o version.o

lesskey_SRCS =	$(lesskey_OBJS:%.o=%.c)

mkhelp_OBJS =	mkhelp.o

mkhelp_SRCS =	$(mkhelp_OBJS:%.o=%.c)

SYSNAME	=	gdamore
RELEASE =	0.9.49
PATCHID =	unbundled

INCS	=	-I .
DEFS 	=	-D SYSNAME=\"$(SYSNAME)\" \
		-D RELEASE=\"$(RELEASE)\" \
		-D VERSION=\"$(PATCHID)\" \
		-D SYSDIR=\"/etc\"

OBJS=		$(less_OBJS) $(lesskey_OBJS) $(compat_OBJS) $(mkhelp_OBJS)

OPT_yes =	$(OPT_$(STD))
DBG_yes =	$(DBG_$(STD))

DBGFLAG =	$(DBG_$(DEBUG))
OPTFLAG =	$(OPT_$(OPTIMIZE))

all:		$(PROGS)

less:	$(less_OBJS) $(compat_OBJS) std
	$(CC) $(DEFS) $(INCS) $(less_OBJS) $(compat_OBJS) -o $@ -lcurses

lesskey:	$(lesskey_OBJS) std
		$(CC) $(DEFS) $(INCS) $(lesskey_OBJS) -o $@

mkhelp:		$(mkhelp_OBJS) std
		$(CC) $(DEFS) $(INCS) $(mkhelp_OBJS) -o $@

more:		less
		$(RM) -f $@
		$(LN) -s less $@

clobber:	clean
		$(RM) $(PROGS) $(PROGS:%=%.lint) help.c morehelp.c

clean:
		$(RM) $(OBJS) $(LINTS)

less.lint:
		$(LINT) $(DEFS) $(INCS) $(less_SRCS) -lcurses -o $@

lesskey.lint:
		$(LINT) $(DEFS) $(INCS) $(lesskey_SRCS) -o $@

mkhelp.lint:
		$(LINT) $(DEFS) $(INCS) $(mkhelp_SRCS) -o $@

more.lint:	less.lint

lint:		$(PROGS:%=%.lint)

less.check:
		$(CSTYLE) $(less_SRCS)

lesskey.check:
		$(CSTYLE) $(lesskey_SRCS)

mkhelp.check:
		$(CSTYLE) $(mkhelp_SRCS)

more.check:

check:		$(PROGS:%=%.check)

%.o: %.c std
		$(CC) $(DEFS) $(INCS) $(CFLAGS) -c -o $@ $<

help.c:		less.hlp mkhelp
		./mkhelp < less.hlp > $@

morehelp.c:	more.hlp mkhelp
		./mkhelp < more.hlp | \
		$(SED) -e s/helpdata/morehelpdata/g > $@

std:
		@if [ -z "$(CC_$(STD))" ]; then  echo "Need to define STD!"; exit 1; fi
