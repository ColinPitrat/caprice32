# makefile for compiling native unix builds
# use "make DEBUG=TRUE" to build a debug executable

LAST_BUILD_IN_DEBUG=$(shell [ -e .debug ] && echo 1 || echo 0)

SOURCES:=$(wildcard *.cpp gui/src/*.cpp)
DEPENDS:=$(SOURCES:.cpp=.d)
OBJECTS:=$(SOURCES:.cpp=.o)

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

$(DEPENDS): %.d: %.cpp
	@echo Computing dependencies for $<
	@$(CXX) -MM $(CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$*.o $*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(OBJECTS): %.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

debug: debug_flag cap32

debug_flag:
ifdef FORCED_DEBUG
	@echo -e '\n!!!!!!!!!!!\n!! Warning: previous build was in debug - rebuilding in debug.\n!! Use make clean before running make to rebuild in release.\n!!!!!!!!!!!\n'
endif
	@touch .debug

cap32: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o cap32 $(OBJECTS) $(LIBS)

clean:
	rm -f $(DEPENDS) $(OBJECTS) cap32 .debug

-include $(DEPENDS)
