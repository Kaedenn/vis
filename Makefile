
# Makefile for the vis project
#
# Nice features:
#
# make <target> EXEC_ARGS="arguments to pass to vis"
# make <target> EXTRA_CFLAGS="extra cflags to pass to gcc"
# make <target> EXTRA_LDFLAGS="extra ldflags to pass to gcc"
# make <target> SCR_ARGS="extra args to pass to process.py"
#

CC = gcc

DIR = .
OBJDIR ?= $(DIR)/.o

SRCS = async.c audio.c clargs.c command.c drawer.c driver.c emit.c emitter.c \
       flist.c forces.c gc.c helper.c kstring.c mutator.c particle.c \
       particle_extra.c plimits.c plist.c random.c script.c genlua.c
SOURCES := $(patsubst %,$(DIR)/%,$(CSRC)) Makefile
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
EXECBIN = vis
VIS := $(DIR)/$(EXECBIN)

TESTS = test/test_kstring test/test_audio

CFLAGS = -fdiagnostics-show-option -ansi \
		 -Wno-unused-variable -Wall -Wextra -Wfloat-equal -Wwrite-strings \
		 -Wshadow -Wpointer-arith -Wcast-qual -Wredundant-decls -Wtrigraphs \
		 -Wswitch-enum -Wundef -Wconversion -pedantic -std=c99
LDFLAGS = -lm

CFLAGS_FAST = -O3 -fexpensive-optimizations
CFLAGS_DEBUG = -O0 -ggdb -DDEBUG=DEBUG_DEBUG
CFLAGS_TRACE = -O0 -ggdb -DDEBUG=DEBUG_TRACE
CFLAGS_PROF = -pg

CFLAGS_LIBS = -I/usr/include/lua5.2 -I/usr/include/SDL2
LDFLAGS_LIBS = -llua5.2 -lSDL2 -lSDL2_image -lSDL2_mixer

CFLAGS := $(CFLAGS) $(CFLAGS_LIBS) $(EXTRA_CFLAGS)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LIBS) $(EXTRA_LDFLAGS)

EXEC_ARGS ?= -i -l $(DIR)/lua/demo_4_random.lua
VG_SUPP := --suppressions=$(DIR)/valgrind.supp
VG_LEAKCHECK := --leak-check=full
VG_REACHABLE := $(VG_LEAKCHECK) --show-reachable=yes --show-leak-kinds=all
VALGRIND_DEFAULT := valgrind $(VG_SUPP) --num-callers=64
VALGRIND := $(VALGRIND_DEFAULT) $(VALGRIND_EXTRA)

SCR_PROCESS = $(DIR)/scripts/process.py
SCR_ARGS ?=

FP_DIR = output
FP_SCRIPT = lua/bowser.lua
FP_BASE ?= $(FP_DIR)/bowser
FP_AUDIO ?= media/bowser-full.mp3
FP_AVI ?= $(FP_DIR)/bowser.avi

.PHONY: all fast debug trace profile execute valgrind leakcheck \
	leakcheck-reachable clean distclean finalproduct fp-prep fp-makeframes \
	fp-flip fp-encode fp-cleanup

all: $(VIS)

fast: $(SOURCES)
	- test -d "$(OBJDIR)/fast" || mkdir -p "$(OBJDIR)/fast"
	$(MAKE) "OBJDIR=$(OBJDIR)/fast" "CFLAGS=$(CFLAGS) $(CFLAGS_FAST)" \
		"$(OBJDIR)/fast/vis"
	rm -f "$(VIS)" && ln "$(OBJDIR)/fast/vis" "$(VIS)"

debug: $(SOURCES)
	- test -d "$(OBJDIR)/debug" || mkdir -p "$(OBJDIR)/debug"
	$(MAKE) "OBJDIR=$(OBJDIR)/debug" "CFLAGS=$(CFLAGS) $(CFLAGS_DEBUG)" \
		"$(OBJDIR)/debug/vis"
	rm -f "$(VIS)" && ln "$(OBJDIR)/debug/vis" "$(VIS)"

trace: $(SOURCES)
	- test -d "$(OBJDIR)/trace" || mkdir -p "$(OBJDIR)/trace"
	$(MAKE) "OBJDIR=$(OBJDIR)/trace" "CFLAGS=$(CFLAGS) $(CFLAGS_TRACE)" \
		"$(OBJDIR)/trace/vis"
	rm -f "$(VIS)" && ln "$(OBJDIR)/trace/vis" "$(VIS)"

profile: $(SOURCES)
	- test -d "$(OBJDIR)/profile" || mkdir -p "$(OBJDIR)/profile"
	$(MAKE) "OBJDIR=$(OBJDIR)/profile" \
		"CFLAGS=$(CFLAGS) $(CFLAGS_FAST) $(CFLAGS_PROF)" \
		"$(OBJDIR)/profile/vis"
	rm -f "$(VIS)" && ln "$(OBJDIR)/profile/vis" "$(VIS)"
	$(VIS) -i -l $(DIR)/lua/demo_5_random.lua
	gprof $(VIS)
	- $(RM) $(DIR)/gmon.out

$(OBJDIR)/%.o: $(DIR)/%.c
	- test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(VIS): $(OBJDIR)/$(VIS)
	rm -f "$@" && ln "$^" "$@"

$(OBJDIR)/$(VIS): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

test/test_kstring: test/test_kstring.c kstring.c helper.c 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 

test/test_audio: test/test_audio.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 

valgrind: debug
	$(VALGRIND) $(VIS) $(EXEC_ARGS)

leakcheck: debug
	$(VALGRIND) $(VG_LEAKCHECK) $(VIS) $(EXEC_ARGS)

leakcheck-reachable: debug
	$(VALGRIND) $(VG_REACHABLE) $(VIS) $(EXEC_ARGS)

clean:
	- $(RM) -r $(OBJDIR)
	- $(RM) $(VIS)

distclean: clean fp-prep

encode:
	python $(SCR_PROCESS) "$(FP_BASE)" "$(FP_AVI)" -i "$(FP_AUDIO)" $(SCR_ARGS)

fp-prep:
	- $(MAKE) fp-cleanup

fp-makeframes: fp-prep
	$(VIS) -l $(FP_SCRIPT) -d $(FP_BASE) -i -q -s 4

fp-encode:
	$(MAKE) encode

fp-cleanup:
	$(RM) $(FP_BASE)_*.png

finalproduct: all fp-prep fp-makeframes fp-encode fp-cleanup

