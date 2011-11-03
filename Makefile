
CSRC = async.c audio.c clargs.c command.c driver.c emitter.c flist.c \
       forces.c frame.c helper.c limits.c particle.c particle_extra.c plist.c \
       random.c script.c
HSRC = async.h audio.h clargs.h command.h defines.h draw.h emitter.h flist.h \
       forces.h frame.h helper.h limits.h particle.h particle_extra.h plist.h \
       random.h script.h
SOURCES = $(CSRC) $(HSRC) Makefile
EXECBIN = vis

CC = kcc -lSDL -lGL -x c99 -bb

all: clean $(SOURCES)
	$(CC) -O -o $(EXECBIN) $(CSRC)

debug: clean $(SOURCES)
	$(CC) -p "-ggdb" -o $(EXECBIN) $(CSRC)

profile: clean $(SOURCES)
	$(CC) -p "-pg" -o $(EXECBIN) $(CSRC)
	rlwrap ./$(EXECBIN) -l script.test.2
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

