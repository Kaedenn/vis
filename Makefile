
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

UNAME_S := $(shell uname -s)

pkg_cflags = $(shell pkg-config --exists $(1) && pkg-config --cflags $(1))
pkg_ldflags = $(shell pkg-config --exists $(1) && pkg-config --libs $(1))

DIR = .
OBJDIR = $(DIR)/.o
DEPDIR = $(DIR)/.d

SRCS := async.c audio.c clargs.c command.c drawer.c driver.c emit.c \
	emitter.c flist.c genlua.c helper.c klist.c kstring.c \
	matrix.c mutator.c particle.c pextra.c plimits.c plist.c \
	random.c script.c shader.c
SOURCES = $(patsubst %,$(DIR)/%,$(CSRC)) Makefile
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPFILES = $(patsubst %.c,$(DEPDIR)/%.d,$(SRCS))
EXECBIN = vis
VIS = $(DIR)/$(EXECBIN)

SRCS := $(SRCS) 3rdparty/miniaudio.c 3rdparty/stb_image_write.c

ifeq ($(UNAME_S),Linux)
SRCS := $(SRCS) audio/pa_latency_probe.c
endif

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

CFLAGS_AUDIO =
LDFLAGS_AUDIO =
ifeq ($(DISABLE_LATENCY_EVAL),)
ifeq ($(UNAME_S),Linux)
CFLAGS_AUDIO = $(call pkg_cflags,libpulse)
LDFLAGS_AUDIO = $(call pkg_ldflags,libpulse)
ifneq ($(CFLAGS_AUDIO),)
CFLAGS_AUDIO += -DHAVE_PULSEAUDIO
endif
endif
endif

CFLAGS_FREETYPE =
LDFLAGS_FREETYPE =
ifeq ($(DISABLE_TEXT),)
CFLAGS_FREETYPE = $(call pkg_cflags,freetype2)
LDFLAGS_FREETYPE = $(call pkg_ldflags,freetype2)
endif

CFLAGS_JSONC = $(call pkg_cflags,json-c)
LDFLAGS_JSONC = $(call pkg_ldflags,json-c)
ifneq ($(CFLAGS_JSONC),)
CFLAGS += -DHAVE_JSONC $(CFLAGS_JSONC)
LDFLAGS += $(LDFLAGS_JSONC)
endif

LDFLAGS_FAST = -O3 -flto
LDFLAGS_PROF = -pg

CFLAGS_LIBS = -I/usr/include/luajit-2.1
LDFLAGS_LIBS = -lluajit-5.1 -lglfw -lGL -lGLEW

CFLAGS := $(CFLAGS) $(CFLAGS_LIBS) $(EXTRA_CFLAGS) $(CFLAGS_AUDIO) $(CFLAGS_FREETYPE)
LDFLAGS := $(LDFLAGS) $(LDFLAGS_LIBS) $(EXTRA_LDFLAGS) $(LDFLAGS_AUDIO) $(LDFLAGS_FREETYPE)

# Determine build mode based on target
ifneq ($(filter fast release,$(MAKECMDGOALS)),)
CFLAGS += $(CFLAGS_FAST)
LDFLAGS += $(LDFLAGS_FAST)
else ifneq ($(filter trace,$(MAKECMDGOALS)),)
CFLAGS += $(CFLAGS_TRACE)
else ifneq ($(filter profile,$(MAKECMDGOALS)),)
CFLAGS += $(CFLAGS_FAST) $(CFLAGS_PROF)
LDFLAGS += $(LDFLAGS_FAST) $(LDFLAGS_PROF)
else
# Default to debug
CFLAGS += $(CFLAGS_DEBUG)
endif

EXEC_ARGS ?= -i -l $(DIR)/lua/demos/demo_4_random.lua
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

all: debug

# If you plan to use lua/letters.lua with LuaJIT or Lua5.1, this is needed
lua/utf8.so: 3rdparty/luautf8-0.2.0.tar.gz
	tar xvfz $^ -C 3rdparty/
	clang -g -fsanitize=fuzzer-no-link,address -fPIC $(CFLAGS_LIBS) 3rdparty/luautf8-0.2.0/lutf8lib.c -shared -o lua/utf8.so

fast release debug trace: $(DEPFILES) $(SOURCES) $(VIS)

profile: $(SOURCES) $(VIS)
	$(VIS) -i -l $(DIR)/lua/demos/demo_5_random.lua
	gprof $(VIS)
	- $(RM) $(DIR)/gmon.out 2>/dev/null

$(DEPDIR): $(SOURCES) $(SCR_MAKEDEP)
	- test -d $(DEPDIR) || mkdir $(DEPDIR) 2>/dev/null
	$(BASH) $(SCR_MAKEDEP)
	touch $(DEPDIR)

CURRENT_FLAGS = $(strip $(CFLAGS) $(LDFLAGS))
OLD_FLAGS := $(strip $(file < .cflags))
ifneq ($(CURRENT_FLAGS),$(OLD_FLAGS))
_DUMMY := $(file > .cflags,$(CURRENT_FLAGS))
endif

$(OBJDIR):
	- test -d $(OBJDIR) || mkdir $(OBJDIR) 2>/dev/null

$(OBJDIR)/3rdparty: | $(OBJDIR)
	- test -d $(OBJDIR)/3rdparty || mkdir $(OBJDIR)/3rdparty 2>/dev/null

$(OBJDIR)/audio: | $(OBJDIR)
	- test -d $(OBJDIR)/audio || mkdir $(OBJDIR)/audio 2>/dev/null

$(DEPFILES): | $(DEPDIR)
$(OBJECTS): | $(OBJDIR)

$(OBJDIR)/3rdparty/%.o: $(DIR)/3rdparty/%.c .cflags | $(OBJDIR)/3rdparty
	$(CC) -c -o $@ $< -msse2 $(CFLAGS) $(CFLAGS_3RDPARTY)

$(OBJDIR)/audio/%.o: $(DIR)/audio/%.c .cflags | $(OBJDIR)/audio
	$(CC) -c -o $@ $< -msse2 $(CFLAGS) $(CFLAGS_AUDIO)

$(OBJDIR)/emitter.o: emitter.c .cflags | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) -Wno-float-equal

$(OBJDIR)/%.o: %.c .cflags | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(VIS): $(OBJECTS) .cflags
	$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

test/test_kstring: test/test_kstring.c kstring.c helper.c
	$(CC) -o $@ $^ $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

test/test_klist: test/test_klist.c klist.c helper.c
	$(CC) -o $@ $^ $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS)

LUA_TEST_TARGETS := $(patsubst $(DIR)/test/%.lua,%,$(LUA_TESTS))
.PHONY: $(LUA_TEST_TARGETS)
$(LUA_TEST_TARGETS): $(VIS)
	$(VIS) -i -l $(DIR)/test/$@.lua

.PHONY: testall
testall: | $(TESTS)
	test -x "$(VIS)" || $(MAKE) debug
	for i in $(LUA_TESTS); do $(VIS) -i -l $$i || exit 1; done
	for i in $(BIN_TESTS); do $$i --automated || exit 1; done

.PHONY: test
test: | $(TESTS)
	test -x "$(VIS)" || $(MAKE) debug
	for i in $(filter-out $(DIR)/test/test_4_kbmouse.lua,$(LUA_TESTS)); do $(VIS) -i -l $$i || exit 1; done
	for i in $(BIN_TESTS); do $$i --automated || exit 1; done

valgrind: debug
	$(VALGRIND) $(VIS) $(EXEC_ARGS)

leakcheck: debug
	$(VALGRIND) $(VG_LEAKCHECK) $(VIS) $(EXEC_ARGS)

leakcheck-reachable: debug
	$(VALGRIND) $(VG_REACHABLE) $(VIS) $(EXEC_ARGS)

miniclean:
	- $(RM) $(OBJDIR)/*.o $(DEPDIR)/*.d 2>/dev/null

clean:
	- $(RM) -r $(DEPDIR) 2>/dev/null
	- $(RM) -r $(OBJDIR) 2>/dev/null
	- $(RM) $(VIS) .cflags 2>/dev/null
	- $(RM) $(BIN_TESTS) 2>/dev/null

distclean: clean fp-prep
	- $(RM) -r 3rdparty/luautf8-0.2.0/ 2>/dev/null

encode:
	python3 $(SCR_PROCESS) "$(FP_BASE)" "$(FP_AVI)" -i "$(FP_AUDIO)" $(SCR_ARGS)

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
