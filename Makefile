
SRCS = async.c audio.c clargs.c command.c drawer.c driver.c emitter.c flist.c \
       forces.c frame.c helper.c limits.c particle.c particle_extra.c plist.c \
       random.c script.c mutator.c types.c
HSRC = async.h audio.h clargs.h command.h defines.h drawer.h emitter.h flist.h \
       forces.h frame.h helper.h limits.h particle.h particle_extra.h plist.h \
       random.h script.h mutator.h types.h
SOURCES = $(CSRC) $(HSRC) Makefile
EXECBIN = vis

PROJECTDIR = $(realpath .)
CFLAGS = -fdiagnostics-show-option -std=c99 \
		 -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		 -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		 -Wswitch-default -Wswitch-enum -Wundef -Wconversion -pedantic
LDFLAGS = -lSDL -lGL -lm

CFLAGS_FAST = -O3 -fexpensive-optimizations
CFLAGS_DEBUG = -O0 -ggdb -DDEBUG -DVIS_SKIP_MANUAL_OPTIMIZATION
CFLAGS_PROF = -pg -DVIS_SKIP_MANUAL_OPTIMIZATION

CFLAGS_LUA = -I/usr/include/lua5.2
LDFLAGS_LUA = -llua5.2

EXEC_ARGS ?= 
VALGRIND = valgrind --suppressions=./valgrind.supp --num-callers=32

CFLAGS := $(CFLAGS) $(CFLAGS_LUA) $(EXTRA_CFLAGS)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LUA) $(EXTRA_LDFLAGS)

all: $(SOURCES)
	$(CC) -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

fast: $(SOURCES)
	$(CC) $(CFLAGS_FAST) -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

debug: $(SOURCES)
	$(CC) $(CFLAGS_DEBUG) -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

profile: $(SOURCES)
	$(CC) $(CFLAGS_PROF) -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)
	rlwrap ./$(EXECBIN) -l test/test.lua
	gprof $(EXECBIN) $(EXEC_ARGS)
	- rm ./gmon.out

execute: $(EXECBIN)
	rlwrap ./$(EXECBIN) $(EXEC_ARGS)

valgrind: debug $(EXECBIN)
	$(VALGRIND) ./$(EXECBIN) $(EXEC_ARGS)

leakcheck: debug $(EXECBIN)
	$(VALGRIND) --leak-check=full ./$(EXECBIN) $(EXEC_ARGS)

leakcheck-reachable: debug $(EXECBIN)
	$(VALGRIND) --leak-check=full --show-reachable=yes --show-leak-kinds=all ./$(EXECBIN) $(EXEC_ARGS)

clean:
	- rm $(EXECBIN)

distclean: clean
