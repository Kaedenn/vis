
SRCS = async.c audio.c clargs.c command.c drawer.c driver.c emitter.c flist.c \
       forces.c helper.c plimits.c particle.c particle_extra.c plist.c \
       random.c script.c mutator.c emit.c kstring.c SDL_SavePNG/savepng.c
SOURCES = $(CSRC) Makefile
EXECBIN = vis

DIR = $(realpath .)
CFLAGS = -fdiagnostics-show-option -std=c99 \
		 -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		 -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		 -Wswitch-default -Wswitch-enum -Wundef -Wconversion -pedantic
LDFLAGS = -lSDL -lGL -lm

CFLAGS_FAST = -O3 -fexpensive-optimizations
CFLAGS_DEBUG = -O0 -ggdb -DDEBUG
CFLAGS_PROF = -pg

CFLAGS_LIBS = -I/usr/include/lua5.2 -I/usr/include/SDL
LDFLAGS_LIBS = -llua5.2 -lpng

EXEC_ARGS ?= 
VALGRIND = valgrind --suppressions=$(DIR)/valgrind.supp --num-callers=32

CFLAGS := $(CFLAGS) $(CFLAGS_LIBS) $(EXTRA_CFLAGS)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LIBS) $(EXTRA_LDFLAGS)

.PHONY: all fast debug profile execute valgrind leakcheck leakcheck-reachable \
	clean distclean

all: $(SOURCES)
	$(CC) -o $(DIR)/$(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

fast: $(SOURCES)
	$(CC) $(CFLAGS_FAST) -o $(DIR)/$(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

debug: $(SOURCES)
	$(CC) $(CFLAGS_DEBUG) -o $(DIR)/$(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

profile: $(SOURCES)
	$(CC) $(CFLAGS_PROF) -o $(DIR)/$(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)
	rlwrap $(DIR)/$(EXECBIN) -l $(DIR)/test/test.lua
	gprof $(DIR)/$(EXECBIN) $(EXEC_ARGS)
	- rm $(DIR)/gmon.out

valgrind: debug $(EXECBIN)
	$(VALGRIND) $(DIR)/$(EXECBIN) $(EXEC_ARGS)

leakcheck: debug $(EXECBIN)
	$(VALGRIND) --leak-check=full $(DIR)/$(EXECBIN) $(EXEC_ARGS)

leakcheck-reachable: debug $(EXECBIN)
	$(VALGRIND) --leak-check=full --show-reachable=yes --show-leak-kinds=all \
		$(DIR)/$(EXECBIN) $(EXEC_ARGS)

clean:
	- rm $(EXECBIN)

distclean: clean
