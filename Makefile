
SRCS = async.c audio.c clargs.c command.c drawer.c driver.c emit.c emitter.c \
       flist.c forces.c gc.c helper.c kstring.c mutator.c particle.c \
       particle_extra.c plimits.c plist.c random.c script.c genlua.c
SOURCES = $(CSRC) Makefile
EXECBIN = vis

DIR = $(realpath .)
CFLAGS = -fdiagnostics-show-option -std=c99 \
		 -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		 -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		 -Wswitch-enum -Wundef -Wconversion -pedantic
LDFLAGS = -lm

CFLAGS_FAST = -O3 -fexpensive-optimizations
CFLAGS_DEBUG = -O0 -ggdb -DDEBUG=3
CFLAGS_TRACE = -O0 -ggdb -DDEBUG=10
CFLAGS_PROF = -pg

CFLAGS_LIBS = -I/usr/include/lua5.2 -I/usr/include/SDL2
LDFLAGS_LIBS = -llua5.2 -lSDL2 -lSDL2_image

CFLAGS := $(CFLAGS) $(CFLAGS_LIBS) $(EXTRA_CFLAGS)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LIBS) $(EXTRA_LDFLAGS)

EXEC_ARGS ?= -i -l $(DIR)/lua/demo_3_random.lua
VALGRIND_DEFAULT = valgrind --suppressions=$(DIR)/valgrind.supp --num-callers=64
VG_LEAKCHECK = --leak-check=full
VG_REACHABLE = $(VG_LEAKCHECK) --show-reachable=yes --show-leak-kinds=all
VALGRIND = $(VALGRIND_DEFAULT) $(VALGRIND_EXTRA)

SCR_FLIP = $(DIR)/scripts/flip.sh
SCR_ENCODE = $(DIR)/scripts/encode.sh
FP_DIR = output
FP_SCRIPT = lua/bowser.lua
FP_BASE1 ?= $(FP_DIR)/bowser1
FP_BASE2 ?= $(FP_DIR)/bowser
FP_AUDIO ?= media/Bowser.wav
FP_AVI ?= $(FP_DIR)/bowser.avi

.PHONY: all fast debug trace profile execute valgrind leakcheck \
	leakcheck-reachable clean distclean finalproduct fp-prep fp-makeframes \
	fp-flip fp-encode fp-cleanup

all: $(SOURCES)
	$(CC) -o $(DIR)/$(EXECBIN) $(SRCS) $(CFLAGS) $(LDFLAGS)

fast: $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST)" all

debug:
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_DEBUG)" all

trace:
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_TRACE)" all

profile: $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST) $(CFLAGS_PROF)" all
	$(DIR)/$(EXECBIN) -i -l $(DIR)/lua/demo_5_random.lua
	gprof $(DIR)/$(EXECBIN)
	- $(RM) $(DIR)/gmon.out

valgrind: debug $(EXECBIN)
	$(VALGRIND) $(DIR)/$(EXECBIN) $(EXEC_ARGS)

leakcheck: debug $(EXECBIN)
	$(VALGRIND) $(VG_LEAKCHECK) $(DIR)/$(EXECBIN) $(EXEC_ARGS)

leakcheck-reachable: debug $(EXECBIN)
	$(VALGRIND) $(VG_REACHABLE) $(DIR)/$(EXECBIN) $(EXEC_ARGS)

clean:
	- $(RM) $(EXECBIN)

distclean: clean fp-prep

encode:
	$(info "make encode FP_BASE=<images> FP_AVI=<output.avi> FP_AUDIO=<song.wav>")
	$(SH) $(SCR_ENCODE) "$(FP_BASE)_%04d.png" $(FP_AUDIO) $(FP_AVI)

fp-prep:
	- $(RM) $(FP_DIR)/bowser*.png 2>/dev/null

fp-makeframes: fp-prep
	$(DIR)/$(EXECBIN) -l $(FP_SCRIPT) -d $(FP_BASE1) -i

fp-flip:
	$(SH) $(SCR_FLIP) "$(FP_BASE1)" "$(FP_BASE2)"

fp-encode:
	make encode "FP_BASE=$(FP_BASE2)"
	#$(SH) $(SCR_ENCODE) "$(FP_BASE2)_%04d.png" $(FP_AUDIO) $(FP_AVI)

fp-cleanup:
	$(RM) $(FP_DIR)/bowser_*.png
	$(RM) $(FP_DIR)/bowser1_*.png

finalproduct: all fp-prep fp-makeframes fp-flip fp-encode fp-cleanup

