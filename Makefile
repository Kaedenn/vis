
# Makefile for the vis project
#
# Nice features:
#
# make <target> EXEC_ARGS="arguments to pass to vis"
# make <target> EXTRA_CFLAGS="extra cflags to pass to gcc"
# make <target> EXTRA_LDFLAGS="extra ldflags to pass to gcc"
# make <target> SCR_ARGS="extra args to pass to process.py"
#

DIR = .
OBJDIR = $(DIR)/o
DEPDIR = $(DIR)/d

SRCS = async.c audio.c clargs.c command.c drawer.c driver.c emit.c emitter.c \
       flist.c forces.c gc.c helper.c kstring.c mutator.c particle.c \
       particle_extra.c plimits.c plist.c random.c script.c genlua.c
SOURCES = $(CSRC) Makefile
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPFILES = $(patsubst %.c,$(DEPDIR)/%.d,$(SRCS))
EXECBIN = vis
VIS = $(DIR)/$(EXECBIN)

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

EXEC_ARGS ?= -i -l $(DIR)/lua/bowser.lua
VG_SUPP = --suppressions=$(DIR)/valgrind.supp
VG_LEAKCHECK = --leak-check=full
VG_REACHABLE = $(VG_LEAKCHECK) --show-reachable=yes --show-leak-kinds=all
VALGRIND_DEFAULT = valgrind $(VG_SUPP) --num-callers=64
VALGRIND = $(VALGRIND_DEFAULT) $(VALGRIND_EXTRA)

SCR_PROCESS = $(DIR)/scripts/process.py
SCR_MAKEDEP = $(DIR)/scripts/makedep.sh
SCR_ARGS ?=

FP_DIR = output
FP_SCRIPT = lua/bowser.lua
FP_BASE ?= $(FP_DIR)/bowser
FP_AUDIO ?= media/Bowser.wav
FP_AVI ?= $(FP_DIR)/bowser.avi

.PHONY: all fast debug trace profile execute valgrind leakcheck \
	leakcheck-reachable clean distclean finalproduct fp-prep fp-makeframes \
	fp-flip fp-encode fp-cleanup

all: $(DEPFILES) $(VIS)

fast: $(DEPFILES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST)" all

debug: $(DEPFILES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_DEBUG)" all

trace: $(DEPFILES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_TRACE)" all

profile: $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST) $(CFLAGS_PROF)" all
	$(VIS) -i -l $(DIR)/lua/demo_5_random.lua
	gprof $(VIS)
	- $(RM) $(DIR)/gmon.out

$(OBJDIR):
	- test -d $(OBJDIR) || mkdir $(OBJDIR) 2>/dev/null

$(DEPDIR)/%.d: $(DEPDIR)
$(DEPFILES): $(DEPDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(DEPDIR): $(SOURCES) $(SCR_MAKEDEP)
	- test -d $(DEPDIR) || mkdir $(DEPDIR) 2>/dev/null
	$(BASH) $(SCR_MAKEDEP)

$(VIS): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

valgrind: debug $(VIS)
	$(VALGRIND) $(VIS) $(EXEC_ARGS)

leakcheck: debug $(VIS)
	$(VALGRIND) $(VG_LEAKCHECK) $(VIS) $(EXEC_ARGS)

leakcheck-reachable: debug $(VIS)
	$(VALGRIND) $(VG_REACHABLE) $(VIS) $(EXEC_ARGS)

clean:
	- $(RM) -r $(DEPDIR)
	- $(RM) -r $(OBJDIR)
	- $(RM) $(VIS)

distclean: clean fp-prep

encode:
	$(info "make encode FP_BASE=<images> FP_AVI=<output.avi> FP_AUDIO=<song.wav>")
	python $(SCR_PROCESS) "$(FP_BASE)" "$(FP_AVI)" -i "$(FP_AUDIO)" $(SCR_ARGS)

fp-prep:
	- $(MAKE) fp-cleanup

fp-makeframes: fp-prep
	$(VIS) -l $(FP_SCRIPT) -d $(FP_BASE) -i -q -s 4 $(EXEC_ARGS)

fp-encode:
	$(MAKE) encode

fp-cleanup:
	$(RM) $(FP_BASE)_*.png

finalproduct: all fp-prep fp-makeframes fp-encode fp-cleanup

include $(DEPFILES)
