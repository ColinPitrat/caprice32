# makefile for compiling native unix builds
# use "make DEBUG=TRUE" to build a debug executable

LAST_BUILD_IN_DEBUG=$(shell [ -e .debug ] && echo 1 || echo 0)

OBJDIR:=obj
SRCDIR:=src
TSTDIR:=test

MAIN:=$(OBJDIR)/main.o

SOURCES:=$(shell find $(SRCDIR) -name \*.cpp)
DEPENDS:=$(foreach file,$(SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
OBJECTS:=$(DEPENDS:.d=.o)

TEST_SOURCES:=$(shell find $(TSTDIR) -name \*.cpp)
TEST_DEPENDS:=$(foreach file,$(TEST_SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
TEST_OBJECTS:=$(TEST_DEPENDS:.d=.o)

IPATHS	= -Isrc/ -Isrc/gui/includes `freetype-config --cflags`
LIBS = `sdl-config --libs` -lz `freetype-config --libs`

.PHONY: all clean debug debug_flag

ifndef CXX
CXX	= g++
endif

COMMON_CFLAGS_1 = -std=c++11
CFLAGS_1	= -Wall -Wzero-as-null-pointer-constant `sdl-config --cflags`

ifndef DEBUG
ifeq ($(LAST_BUILD_IN_DEBUG), 1)
FORCED_DEBUG=1
DEBUG=1
endif
endif

ifdef DEBUG
COMMON_CFLAGS_2	= $(COMMON_CFLAGS_1) -g -Og
all: debug
else
COMMON_CFLAGS_2	= $(COMMON_CFLAGS_1) -O2 -funroll-loops -ffast-math -fomit-frame-pointer -fno-strength-reduce -finline-functions -s
all: cap32
endif

ifdef WITHOUT_GL
CFLAGS_2 = $(CFLAGS_1)
else
CFLAGS_2 = $(CFLAGS_1) -DHAVE_GL
endif

COMMON_CFLAGS=$(COMMON_CFLAGS_2)
CFLAGS = $(COMMON_CFLAGS) $(CFLAGS_2) $(IPATHS)

$(MAIN): main.cpp src/cap32.h
	@$(CXX) -c $(CFLAGS) -o $(MAIN) main.cpp

$(DEPENDS): $(OBJDIR)/%.d: %.cpp
	@echo Computing dependencies for $<
	@mkdir -p `dirname $@`
	@$(CXX) -MM $(CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$(OBJDIR)/$*.o $(OBJDIR)/$*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(OBJECTS): $(OBJDIR)/%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

debug: debug_flag cap32 unit_test debug_flag

debug_flag:
ifdef FORCED_DEBUG
	@echo -e '\n!!!!!!!!!!!\n!! Warning: previous build was in debug - rebuilding in debug.\n!! Use make clean before running make to rebuild in release.\n!!!!!!!!!!!\n'
endif
	@touch .debug

cap32: $(OBJECTS) $(MAIN)
	$(CXX) $(LDFLAGS) -o cap32 $(OBJECTS) $(MAIN) $(LIBS)

####################################
### Tests
####################################

googletest:
	@[ -d googletest ] || git clone https://github.com/google/googletest.git

TEST_CFLAGS=$(COMMON_CFLAGS) -I$(GTEST_DIR)/include -I$(GTEST_DIR)
TEST_TARGET=$(TSTDIR)/test_runner
GTEST_DIR=googletest/googletest/

$(TEST_DEPENDS): $(OBJDIR)/%.d: %.cpp
	@echo Computing dependencies for $<
	@mkdir -p `dirname $@`
	@$(CXX) -MM $(TEST_CFLAGS) $(IPATHS) $< | { sed 's#^[^:]*\.o[ :]*#$(OBJDIR)/$*.o $(OBJDIR)/$*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(TEST_OBJECTS): $(OBJDIR)/%.o: %.cpp googletest
	$(CXX) -c $(TEST_CFLAGS) $(IPATHS) -o $@ $<

$(GTEST_DIR)/src/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc googletest
	$(CXX) $(TEST_CFLAGS) -c $(INCPATH) -o $@ $<

unit_test: $(OBJECTS) $(TEST_OBJECTS) $(GTEST_DIR)/src/gtest-all.o
	$(CXX) $(TEST_CFLAGS) -o $(TEST_TARGET) $(GTEST_DIR)/src/gtest-all.o $(TEST_OBJECTS) $(OBJECTS) $(LIBS) 
	./$(TEST_TARGET) --gtest_shuffle

clean:
	rm -rf $(OBJDIR)
	rm -f $(TEST_TARGET) $(GTEST_DIR)/src/gtest-all.o cap32 .debug

-include $(DEPENDS) $(TEST_DEPENDS)
