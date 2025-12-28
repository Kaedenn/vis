
# Makefile for the vis project
#
# Extra directives:
#   EXEC_ARGS		arguments to pass to vis
#   EXTRA_CFLAGS	additional compilation flags to pass to gcc
#   EXTRA_LDFLAGS	additional linker flags to pass to ld
#   SCR_ARGS		additional arguments to pass to process.py
#
# Note that setting CFLAGS or LDFLAGS will override their default values.
# Use EXTRA_CFLAGS or EXTRA_LDFLAGS instead.
#

DIR = .
OBJDIR = $(DIR)/.o
DEPDIR = $(DIR)/.d

SRCS := async.c audio.c clargs.c command.c drawer.c driver.c emit.c \
	emitter.c flist.c forces.c genlua.c helper.c klist.c kstring.c \
	kstring.c mutator.c particle.c pextra.c plimits.c plist.c \
	random.c script.c shader.c
SOURCES = $(patsubst %,$(DIR)/%,$(CSRC)) Makefile
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPFILES = $(patsubst %.c,$(DEPDIR)/%.d,$(SRCS))
EXECBIN = vis
VIS = $(DIR)/$(EXECBIN)

SRCS := $(SRCS) 3rdparty/miniaudio.c 3rdparty/stb_image_write.c

LUA_TESTS := $(wildcard $(DIR)/test/test_*.lua)
C_TESTS := $(wildcard $(DIR)/test/test_*.c)
BIN_TESTS := $(patsubst %.c,%,$(C_TESTS))
TESTS := $(LUA_TESTS) $(BIN_TESTS)

CFLAGS ?= -Wall -Wextra -Wfloat-equal -Wwrite-strings -Wshadow -Wpointer-arith \
	-Wcast-qual -Wredundant-decls -Wswitch-enum -Wundef -Wconversion -ansi \
	-pedantic -std=c99 -fdiagnostics-show-option

LDFLAGS ?= -lm -ldl -lpthread -pthread

CFLAGS_FAST = -O3 -fexpensive-optimizations -flto
CFLAGS_DEBUG = -O0 -ggdb -DDEBUG=DEBUG_DEBUG
CFLAGS_TRACE = -O0 -ggdb -DDEBUG=DEBUG_TRACE
CFLAGS_PROF = -pg

CFLAGS_3RDPARTY = -Wno-conversion -Wno-switch-enum -Wno-cast-qual \
				  -Wno-float-equal -Wno-shadow

LDFLAGS_FAST = -O3 -flto
LDFLAGS_PROF = -pg

CFLAGS_LIBS = -I/usr/include/lua5.2
LDFLAGS_LIBS = -llua5.2 -lglfw -lGL -lGLEW

CFLAGS := $(CFLAGS) $(CFLAGS_LIBS) $(EXTRA_CFLAGS)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LIBS) $(EXTRA_LDFLAGS)

ifneq ($(DEBUG),)
all: CFLAGS += $(CFLAGS_DEBUG)
endif

EXEC_ARGS ?= -i -l $(DIR)/lua/demo_4_random.lua
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
FP_AUDIO ?= media/bowser-full.mp3
FP_AVI ?= $(FP_DIR)/bowser.avi

#.PHONY: all fast debug trace profile execute valgrind leakcheck \
#	leakcheck-reachable clean distclean finalproduct fp-prep fp-makeframes \
#	fp-flip fp-encode fp-cleanup test

all: debug

fast: $(DEPFILES) $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST)" \
		"LDFLAGS=$(LDFLAGS) $(LDFLAGS_FAST)" $(VIS)

release: $(DEPFILES) $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS)" $(VIS)

debug: $(DEPFILES) $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_DEBUG)" $(VIS)

trace: $(DEPFILES) $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_TRACE)" $(VIS)

profile: $(SOURCES)
	$(MAKE) "CFLAGS=$(CFLAGS) $(CFLAGS_FAST) $(CFLAGS_PROF)" \
		"LDFLAGS=$(LDFLAGS) $(LDFLAGS_FAST) $(LDFLAGS_PROF)" $(VIS)
	$(VIS) -i -l $(DIR)/lua/demo_5_random.lua
	gprof $(VIS)
	- $(RM) $(DIR)/gmon.out 2>/dev/null

$(DEPDIR): $(SOURCES) $(SCR_MAKEDEP)
	- test -d $(DEPDIR) || mkdir $(DEPDIR) 2>/dev/null
	$(BASH) $(SCR_MAKEDEP)
	touch $(DEPDIR)

$(OBJDIR):
	- test -d $(OBJDIR) || mkdir $(OBJDIR) 2>/dev/null

$(OBJDIR)/3rdparty: | $(OBJDIR)
	- test -d $(OBJDIR)/3rdparty || mkdir $(OBJDIR)/3rdparty 2>/dev/null

$(DEPFILES): | $(DEPDIR)
$(OBJECTS): | $(OBJDIR)

$(OBJDIR)/3rdparty/%.o: $(DIR)/3rdparty/%.c | $(OBJDIR)/3rdparty
	$(CC) -c -o $@ $< -msse2 $(CFLAGS) $(CFLAGS_3RDPARTY)

$(OBJDIR)/emitter.o: emitter.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) -Wno-float-equal

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(VIS): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

test/test_kstring: test/test_kstring.c kstring.c helper.c
	$(CC) -o $@ $^ $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

test/test_klist: test/test_klist.c klist.c helper.c
	$(CC) -o $@ $^ $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

test/test_audio: test/test_audio.c
	$(CC) -o $@ $^ $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

test: $(TESTS)
	test -x "$(VIS)" || $(MAKE) debug
	for i in $(LUA_TESTS); do $(VIS) -i -l $$i || exit 1; done
	for i in $(BIN_TESTS); do $$i --automated || exit 1; done

valgrind: debug
	$(VALGRIND) $(VIS) $(EXEC_ARGS)

leakcheck: debug
	$(VALGRIND) $(VG_LEAKCHECK) $(VIS) $(EXEC_ARGS)

leakcheck-reachable: debug
	$(VALGRIND) $(VG_REACHABLE) $(VIS) $(EXEC_ARGS)

clean:
	- $(RM) -r $(DEPDIR) 2>/dev/null
	- $(RM) -r $(OBJDIR) 2>/dev/null
	- $(RM) $(VIS) 2>/dev/null
	- $(RM) $(BIN_TESTS) 2>/dev/null

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
	$(RM) $(FP_BASE)_*.png 2>/dev/null

finalproduct: all fp-prep fp-makeframes fp-encode fp-cleanup

include $(DEPFILES)
