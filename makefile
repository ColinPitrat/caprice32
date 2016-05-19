# makefile for compiling native unix builds
# use "make DEBUG=TRUE" to build a debug executable

LAST_BUILD_IN_DEBUG=$(shell [ -e .debug ] && echo 1 || echo 0)

SOURCES:=$(wildcard *.cpp gui/src/*.cpp)
DEPENDS:=$(SOURCES:.cpp=.d)
OBJECTS:=$(SOURCES:.cpp=.o)

TEST_SOURCES:=$(shell find test -name \*.cpp)
TEST_DEPENDS:=$(TEST_SOURCES:.cpp=.d)
TEST_OBJECTS:=$(TEST_SOURCES:.cpp=.o)

IPATHS	= -I. -Igui/includes `freetype-config --cflags`
LIBS = `sdl-config --libs` -lz `freetype-config --libs`

.PHONY: all clean debug debug_flag

ifndef CXX
CXX	= g++
endif

CFLAGS_1	= -std=c++11 -Wall -Wzero-as-null-pointer-constant `sdl-config --cflags`

ifndef DEBUG
ifeq ($(LAST_BUILD_IN_DEBUG), 1)
FORCED_DEBUG=1
DEBUG=1
endif
endif

ifdef DEBUG
CFLAGS_2	= $(CFLAGS_1) -g -Og
all: debug
else
CFLAGS_2	= $(CFLAGS_1) -O2 -funroll-loops -ffast-math -fomit-frame-pointer -fno-strength-reduce -finline-functions -s
all: cap32
endif

ifdef WITHOUT_GL
CFLAGS_3 = $(CFLAGS_2)
else
CFLAGS_3 = $(CFLAGS_2) -DHAVE_GL
endif

CFLAGS = $(CFLAGS_3) $(IPATHS)

$(DEPENDS) $(TEST_DEPENDS): %.d: %.cpp
	@echo Computing dependencies for $<
	@$(CXX) -MM $(CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$*.o $*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(OBJECTS): %.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

$(TEST_OBJECTS): %.o: %.cpp gtest
	$(CXX) -c $(CFLAGS) $(TEST_CFLAGS) -o $@ $<

debug: debug_flag cap32 unit_test

debug_flag:
ifdef FORCED_DEBUG
	@echo -e '\n!!!!!!!!!!!\n!! Warning: previous build was in debug - rebuilding in debug.\n!! Use make clean before running make to rebuild in release.\n!!!!!!!!!!!\n'
endif
	@touch .debug

cap32: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o cap32 $(OBJECTS) $(LIBS)

####################################
### Tests
####################################

gtest:
	[ -d googletest ] || git clone https://github.com/google/googletest.git

TEST_TARGET=test/unit/unit_tester
GTEST_DIR=googletest/googletest/
TEST_CFLAGS=-I$(GTEST_DIR)/include -I$(GTEST_DIR)

$(GTEST_DIR)/src/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc gtest
	$(CXX) $(TEST_CFLAGS) -c $(INCPATH) -o $@ $<

unit_test: $(OBJECTS) $(TEST_OBJECTS) $(GTEST_DIR)/src/gtest-all.o
	$(CXX) $(IPATHS) $(TEST_CFLAGS) -o $(TEST_TARGET) $(LIBS) $(GTEST_DIR)/src/gtest-all.o $(OBJECTS) $(TEST_OBJECTS)
	./$(TEST_TARGET) --gtest_shuffle

clean:
	rm -f $(DEPENDS) $(OBJECTS) $(TEST_TARGET) $(GTEST_DIR)/src/gtest-all.o cap32 .debug

-include $(DEPENDS)
