#
# nccc Makefile
#

CC      	= gcc
CFLAGS_LIB	= -ansi -pedantic -Wall -O2 -c
CFLAGS  	= -ansi -pedantic -Wall -O2 -g
LFLAGS  	= -Iinclude -L. -liniparser
AR		= ar
ARFLAGS 	= rcv
RM      	= rm -f


# Implicit rules

SUFFIXES 	= .o .c .h .a .so .sl

LIB_COMPILE.c= $(CC) -fPIC $(CFLAGS_LIB)

.c.o:
	@(echo "compiling $< ...")
	@($(LIB_COMPILE.c) -o $@ $<)

INIPARSER_SRCS 	= include/iniparser.c \
	   	include/dictionary.c \
	   	include/strlib.c

INIPARSER_OBJS 	= $(INIPARSER_SRCS:.c=.o)

SNL_SRC		= include/snl.c

default:	all

all:		nccc

libiniparser.a:	$(INIPARSER_OBJS)
		@($(AR) $(ARFLAGS) libiniparser.a $(INIPARSER_OBJS)) \
		&& ranlib libiniparser.a

snl.o:		include/snl.c include/snl.h
		$(CC) $(CFLAGS_LIB) -o include/snl.o include/snl.c

chprint.o:	libiniparser.a core/chprint.c core/chprint.h
		$(CC) $(CFLAGS_LIB) -Iinclude -o core/chprint.o core/chprint.c

config.o:	libiniparser.a core/config.c core/config.h
		$(CC) $(CFLAGS_LIB) -Iinclude -o core/config.o core/config.c

nccc:		chprint.o config.o core/nccc.c
		$(CC) $(CFLAGS) -o nccc core/chprint.o core/config.o \
		core/nccc.c $(LFLAGS)

clean:
		$(RM) core/chprint.o core/config.o nccc

distclean:
		$(RM) $(INIPARSER_OBJS) libiniparser.* core/chprint.o corenccc


docs:
		@(cd doc ; $(MAKE))
	
check:
		@(cd test ; $(MAKE))

renew:		
		make clean && make all
