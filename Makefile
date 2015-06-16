
SRCS = async.c audio.c clargs.c command.c driver.c emitter.c flist.c \
       forces.c frame.c helper.c limits.c particle.c particle_extra.c plist.c \
       random.c scheduler.c script.c mutator.c
HSRC = async.h audio.h clargs.h command.h defines.h draw.h emitter.h flist.h \
       forces.h frame.h helper.h limits.h particle.h particle_extra.h plist.h \
       random.h scheduler.h script.h mutator.h
SOURCES = $(CSRC) $(HSRC) Makefile
EXECBIN = vis

PROJECTDIR = $(realpath .)

CXXFLAGS = -fdiagnostics-show-option -fexpensive-optimizations -O3 \
		   -DVIS_SKIP_MANUAL_OPTIMIZATION -std=c99 -Isquirrel \
		   -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		   -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		   -Wswitch-default -Wswitch-enum -Wundef -Wconversion -pedantic
LDFLAGS = -Llibsquirrel/lib -lSDL -lGL -lsquirrel -lsqstdlib -lm -lstdc++

all: $(SOURCES)
	$(CC) -o $(EXECBIN) $(SRCS) $(CXXFLAGS) $(LDFLAGS)

debug: $(SOURCES)
	$(CC) -ggdb -o $(EXECBIN) $(SRCS) $(CXXFLAGS) $(LDFLAGS)

profile: $(SOURCES)
	$(CC) -pg -o $(EXECBIN) $(SRCS) $(CXXFLAGS) $(LDFLAGS)
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
