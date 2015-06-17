
SRCS = async.c audio.c clargs.c command.c driver.c emitter.c flist.c \
       forces.c frame.c helper.c limits.c particle.c particle_extra.c plist.c \
       random.c scheduler.c script.c mutator.c
HSRC = async.h audio.h clargs.h command.h defines.h draw.h emitter.h flist.h \
       forces.h frame.h helper.h limits.h particle.h particle_extra.h plist.h \
       random.h scheduler.h script.h mutator.h
SOURCES = $(CSRC) $(HSRC) Makefile
EXECBIN = vis

PROJECTDIR = $(realpath .)

CFLAGS = -fdiagnostics-show-option -std=c99 -Isquirrel -I/usr/include/lua5.2 \
		 -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		 -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		 -Wswitch-default -Wswitch-enum -Wundef -Wconversion -pedantic \
		 -DVIS_USE_LUA=1
LDFLAGS = -Llibsquirrel/lib -lSDL -lGL -lsquirrel -lsqstdlib -lm -lstdc++ -llua5.2

all: $(SOURCES)
	$(CC) -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

fast: $(SOURCES)
	$(CC) -O3 -fexpensive-optimizations -DVIS_SKIP_MANUAL_OPTIMIZATION -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

debug: $(SOURCES)
	$(CC) -ggdb -DDEBUG=1 -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

profile: $(SOURCES)
	$(CC) -pg -o $(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)
	rlwrap ./$(EXECBIN) -l script.nut
	gprof $(EXECBIN)
	- rm ./gmon.out

execute: all $(EXECBIN)
	rlwrap ./$(EXECBIN)

ex: execute

valgrind: debug $(EXECBIN)
	valgrind ./$(EXECBIN)

leakcheck: debug $(EXECBIN)
	valgrind --leak-check=full --show-reachable=yes ./$(EXECBIN)

clean:
	- rm $(EXECBIN)

distclean: clean
