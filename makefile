# use "make" to build for linux
# use "make debug" or "make DEBUG=TRUE" to build a debug executable for linux
# use "make ARCH=win32" or "make ARCH=win64" to build for windows
# Interesting targets (in addition to the default 'all'):
#  - debug
#  - clean
#  - distrib
#  - doc
# Supported variables:
#  - ARCH = (linux|win32|win64)
#  - CXX (default = g++)
#  - CFLAGS
#  - LDFLAGS
#  - WITHOUT_GL
#  - WITH_IPF

LAST_BUILD_IN_DEBUG = $(shell [ -e .debug ] && echo 1 || echo 0)
GIT_HASH = $(shell git rev-parse --verify HEAD)
# If compiling under native windows, set WINE to ""
WINE = wine

ifndef ARCH
ARCH = linux
endif

ifeq ($(ARCH),win64)
TRIPLE = x86_64-w64-mingw32
TARGET=cap32.exe
TEST_TARGET = test_runner.exe
PLATFORM=windows
CAPSIPFDLL=CAPSImg_x64.dll
else ifeq ($(ARCH),win32)
TRIPLE = i686-w64-mingw32
TARGET=cap32.exe
TEST_TARGET = test_runner.exe
PLATFORM=windows
CAPSIPFDLL=CAPSImg.dll
else ifeq ($(ARCH),linux)
TARGET=cap32
TEST_TARGET = test_runner
PLATFORM=linux
else
$(error Unknown ARCH. Supported ones are linux, win32 and win64.)
endif

ifeq ($(PLATFORM),windows)
MINGW_PATH = /usr/$(TRIPLE)
IPATHS = -Isrc/ -Isrc/gui/includes -I$(MINGW_PATH)/include -I$(MINGW_PATH)/include/SDL -I$(MINGW_PATH)/include/freetype2
LIBS = $(MINGW_PATH)/lib/libSDL.dll.a $(MINGW_PATH)/lib/libfreetype.dll.a $(MINGW_PATH)/lib/libz.dll.a $(MINGW_PATH)/lib/libpng16.dll.a $(MINGW_PATH)/lib/libpng.dll.a
COMMON_CFLAGS = -DWINDOWS
CXX = $(TRIPLE)-g++
ifdef WITH_IPF
COMMON_CFLAGS += -DWITH_IPF
LIBS += $(MINGW_PATH)/bin/$(CAPSIPFDLL)
endif
else
IPATHS = -Isrc/ -Isrc/gui/includes `freetype-config --cflags` `sdl-config --cflags` `pkg-config --cflags libpng`
LIBS = `sdl-config --libs` -lz `freetype-config --libs` `pkg-config --libs libpng`
ifdef WITH_IPF
COMMON_CFLAGS += -DWITH_IPF
LIBS += -lcapsimage
endif
ifndef CXX
CXX = g++
endif
endif

SRCDIR:=src
TSTDIR:=test
OBJDIR:=obj/$(ARCH)
ARCHIVE = release/cap32-$(ARCH)

HTML_DOC:=doc/man.html
GROFF_DOC:=doc/man6/cap32.6

MAIN:=$(OBJDIR)/main.o

SOURCES:=$(shell find $(SRCDIR) -name \*.cpp)
DEPENDS:=$(foreach file,$(SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
OBJECTS:=$(DEPENDS:.d=.o)

TEST_SOURCES:=$(shell find $(TSTDIR) -name \*.cpp)
TEST_DEPENDS:=$(foreach file,$(TEST_SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
TEST_OBJECTS:=$(TEST_DEPENDS:.d=.o)

.PHONY: all check_deps clean debug debug_flag distrib doc insert_hash unit_test

WARNINGS = -Wall -Wextra -Wzero-as-null-pointer-constant -Wformat=2 -Wold-style-cast -Wmissing-include-dirs -Wlogical-op -Woverloaded-virtual -Wpointer-arith -Wredundant-decls
COMMON_CFLAGS += $(CFLAGS) -std=c++11 $(IPATHS)
DEBUG_FLAGS = -Werror -g -O0 -DDEBUG
RELEASE_FLAGS = -O2 -funroll-loops -ffast-math -fomit-frame-pointer -fno-strength-reduce -finline-functions -s
BUILD_FLAGS = $(RELEASE_FLAGS)

debug: BUILD_FLAGS:=$(DEBUG_FLAGS)

ifndef DEBUG
ifeq ($(LAST_BUILD_IN_DEBUG), 1)
FORCED_DEBUG = 1
DEBUG = 1
endif
endif

ifndef WITHOUT_GL
COMMON_CFLAGS += -DHAVE_GL
endif

ifdef DEBUG
BUILD_FLAGS = $(DEBUG_FLAGS)
all: insert_hash check_deps debug
else
all: insert_hash check_deps distrib
endif

# gtest doesn't build with warnings flags, hence the COMMON_CFLAGS
ALL_CFLAGS=$(COMMON_CFLAGS) $(WARNINGS)

src/argparse.c: insert_hash

$(MAIN): main.cpp src/cap32.h
	@$(CXX) -c $(BUILD_FLAGS) $(ALL_CFLAGS) -o $(MAIN) main.cpp

$(DEPENDS): $(OBJDIR)/%.d: %.cpp
	@echo Computing dependencies for $<
	@mkdir -p `dirname $@`
	@$(CXX) -MM $(BUILD_FLAGS) $(ALL_CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$(OBJDIR)/$*.o $(OBJDIR)/$*.os $(OBJDIR)/$*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(OBJECTS): $(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) -c $(BUILD_FLAGS) $(ALL_CFLAGS) -o $@ $<

debug: debug_flag tags distrib unit_test

debug_flag:
ifdef FORCED_DEBUG
	@echo -e '\n!!!!!!!!!!!\n!! Warning: previous build was in debug - rebuilding in debug.\n!! Use make clean before running make to rebuild in release.\n!!!!!!!!!!!\n'
endif
	@touch .debug

ifeq ($(PLATFORM),linux)
check_deps:
	@sdl-config --cflags >/dev/null 2>&1 || (echo "Error: missing dependency libsdl-1.2. Try installing libsdl 1.2 development package (e.g: libsdl1.2-dev)" && false)
	@freetype-config --cflags >/dev/null 2>&1 || (echo "Error: missing dependency libfreetype. Try installing libfreetype development package (e.g: libfreetype6-dev)" && false)
	@pkg-config --cflags zlib >/dev/null 2>&1 || (echo "Error: missing dependency zlib. Try installing zlib development package (e.g: zlib-devel)" && false)
	@pkg-config --cflags libpng >/dev/null 2>&1 || (echo "Error: missing dependency libpng. Try installing libpng development package (e.g: libpng-devel)" && false)
else
# TODO(cpitrat): Implement check_deps for windows build
check_deps:
endif

# This might fail on non GNU systems as sed -i in GNU sed only
insert_hash:
	@sed -i 's/commit_hash = ".*"/commit_hash = "$(GIT_HASH)"/' src/argparse.cpp

tags:
	@ctags -R . || echo -e "!!!!!!!!!!!\n!! Warning: ctags not found - if you are a developer, you might want to install it.\n!!!!!!!!!!!"

doc: $(HTML_DOC)

$(HTML_DOC): $(GROFF_DOC)
	groff -mandoc -Thtml $< > $@

$(TARGET): $(OBJECTS) $(MAIN)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN) $(LIBS)

# TODO(cpitrat): Make it work for linux too
ifeq ($(PLATFORM),windows)
distrib: $(TARGET)
	mkdir -p $(ARCHIVE)
	rm -f $(ARCHIVE).zip
	cp $(TARGET) $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/SDL.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libbz2-1.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libfreetype-6.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libgcc_s_*-1.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libpng16-16.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libstdc++-6.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/libwinpthread-1.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/zlib1.dll $(ARCHIVE)/
	cp $(MINGW_PATH)/bin/$(CAPSIPFDLL) $(ARCHIVE)/CAPSImg.dll
	cp cap32.cfg $(ARCHIVE)/
	cp -r resources/ rom/ $(ARCHIVE)/
	zip -r $(ARCHIVE).zip $(ARCHIVE)
else
distrib: $(TARGET)
endif

####################################
### Tests
####################################

googletest:
	@[ -d googletest ] || git clone https://github.com/google/googletest.git

TEST_CFLAGS = $(COMMON_CFLAGS) -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_DIR = googletest/googletest/

$(TEST_DEPENDS): $(OBJDIR)/%.d: %.cpp
	@echo Computing dependencies for $<
	@mkdir -p `dirname $@`
	@$(CXX) -MM $(BUILD_FLAGS) $(TEST_CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$(OBJDIR)/$*.o $(OBJDIR)/$*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(TEST_OBJECTS): $(OBJDIR)/%.o: %.cpp googletest
	$(CXX) -c $(BUILD_FLAGS) $(TEST_CFLAGS) -o $@ $<

$(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc googletest
	@mkdir -p `dirname $@`
	$(CXX) -c $(BUILD_FLAGS) $(TEST_CFLAGS) -o $@ $<

$(TEST_TARGET): $(OBJECTS) $(TEST_OBJECTS) $(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o
	$(CXX) $(LDFLAGS) -o $(TEST_TARGET) $(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o $(TEST_OBJECTS) $(OBJECTS) $(LIBS) -lpthread

ifeq ($(PLATFORM),windows)
unit_test: $(TEST_TARGET)
	cp $(TEST_TARGET) $(ARCHIVE)/
	rm -fr $(ARCHIVE)/test
	ln -s -f ../../test $(ARCHIVE)/test
	cd $(ARCHIVE) && $(WINE) ./$(TEST_TARGET) --gtest_shuffle
else
unit_test: $(TEST_TARGET)
	./$(TEST_TARGET) --gtest_shuffle
endif

clean:
	rm -rf obj/ release/
	rm -f test_runner test_runner.exe cap32 cap32.exe .debug tags

-include $(DEPENDS) $(TEST_DEPENDS)
