# use "make" to build for linux
# use "make debug" or "make DEBUG=TRUE" to build a debug executable for linux
# use "make ARCH=win32" or "make ARCH=win64" to build for windows
# Interesting targets (in addition to the default 'all'):
#  - debug
#  - clean
#  - distrib
#  - doc
# Supported variables:
#  - ARCH = (linux|win32|win64|macos)
#  - CXX (default = g++)
#  - CFLAGS
#  - LDFLAGS
#  - WITHOUT_GL

# To be overridden for debian packaging
VERSION=latest
REVISION=0

LAST_BUILD_IN_DEBUG = $(shell [ -e .debug ] && echo 1 || echo 0)

ARCH ?= linux

COMMON_CFLAGS ?= 

ifeq ($(ARCH),win64)
# Rename main to SDL_main to solve the "undefined reference to `SDL_main'".
# Do not make an error of old-style-cast on msys2 as the version of GCC used by
# msys2 on GitHub actions is 13.3 which has a bug and raise it on a cast from
# zlib.h
COMMON_CFLAGS = -DWINDOWS -D_POSIX_C_SOURCE=200809L -Wno-error=old-style-cast
PLATFORM=windows
MINGW_PATH=/mingw64
else ifeq ($(ARCH),win32)
COMMON_CFLAGS = -DWINDOWS -D_POSIX_C_SOURCE=200809L -Wno-error=old-style-cast
PLATFORM=windows
MINGW_PATH=/mingw32
else ifeq ($(ARCH),linux)
PLATFORM=linux
else ifeq ($(ARCH),macos)
# Yes that's weird, but the build on macos works the same way as on linux
PLATFORM=linux
else
$(error Unknown ARCH. Supported ones are linux, win32 and win64.)
endif

ifeq ($(PLATFORM),windows)
TARGET = cap32.exe
TEST_TARGET = test_runner.exe
COMMON_CFLAGS += -DWINDOWS
else
prefix = /usr/local
TARGET = cap32
TEST_TARGET = test_runner
endif

CAPS_INCLUDES=-Isrc/capsimg/LibIPF -Isrc/capsimg/Device -Isrc/capsimg/CAPSImg -Isrc/capsimg/Codec -Isrc/capsimg/Core

IPATHS = -Isrc/ $(CAPS_INCLUDES) -Isrc/gui/includes `pkg-config --cflags freetype2` `sdl2-config --cflags` `pkg-config --cflags libpng` `pkg-config --cflags zlib`
LIBS = `sdl2-config --libs` `pkg-config --libs freetype2` `pkg-config --libs libpng` `pkg-config --libs zlib`
CXX ?= g++
COMMON_CFLAGS += -fPIC

ifneq (,$(findstring g++,$(CXX)))
LIBS += -lstdc++fs
endif

ifndef RELEASE
GIT_HASH = $(shell git rev-parse --verify HEAD)
COMMON_CFLAGS += -DHASH=\"$(GIT_HASH)\"
endif

ifdef APP_PATH
COMMON_CFLAGS += -DAPP_PATH=\"$(APP_PATH)\"
else
$(info Notice: APP_PATH not specified.  Will look for cap32.cfg debug-style.  See `README.md` for details. )
endif

ifdef DESTDIR
COMMON_CFLAGS += -DDESTDIR=\"$(DESTDIR)\"
endif

CLANG_FORMAT=clang-format
CLANG_TIDY=clang-tidy
CLANG_CHECKS=modernize-*,performance-*,misc-*,readability-*,-misc-definitions-in-headers,-misc-non-private-member-variables-in-classes,-misc-no-recursion,-modernize-avoid-c-arrays,-modernize-deprecated-headers,-modernize-make-unique,-modernize-use-auto,-modernize-use-default-member-init,-modernize-use-equals-delete,-modernize-use-nodiscard,-modernize-use-trailing-return-type,-modernize-use-using,-performance-unnecessary-value-param,-readability-avoid-const-params-in-decls,-readability-braces-around-statements,-readability-convert-member-functions-to-static,-readability-function-size,-readability-implicit-bool-cast,-readability-implicit-bool-conversion,-readability-isolate-declaration,-readability-magic-numbers,-readability-qualified-auto,-readability-uppercase-literal-suffix,-readability-use-anyofallof

SRCDIR:=src
TSTDIR:=test
OBJDIR:=obj/$(ARCH)
RELEASE_DIR = release
ARCHIVE = cap32-$(ARCH)
ARCHIVE_DIR = $(RELEASE_DIR)/$(ARCHIVE)

HTML_DOC:=doc/man.html
GROFF_DOC:=doc/man6/cap32.6

MAIN:=$(OBJDIR)/main.o

SOURCES:=$(shell find $(SRCDIR) -name \*.cpp)
HEADERS:=$(shell find $(SRCDIR) -name \*.h)
DEPENDS:=$(foreach file,$(SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
OBJECTS:=$(DEPENDS:.d=.o)

TEST_SOURCES:=$(shell find $(TSTDIR) -name \*.cpp)
TEST_HEADERS:=$(shell find $(TSTDIR) -name \*.h)
TEST_DEPENDS:=$(foreach file,$(TEST_SOURCES:.cpp=.d),$(shell echo "$(OBJDIR)/$(file)"))
TEST_OBJECTS:=$(TEST_DEPENDS:.d=.o)

.PHONY: all check_deps clean deb_pkg debug debug_flag distrib doc tags unit_test install doxygen

WARNINGS = -Wall -Wextra -Wzero-as-null-pointer-constant -Wformat=2 -Wold-style-cast -Wmissing-include-dirs -Woverloaded-virtual -Wpointer-arith -Wredundant-decls
COMMON_CFLAGS += $(CFLAGS) -std=c++17 $(IPATHS)
DEBUG_FLAGS = -Werror -g -O0 -DDEBUG
RELEASE_FLAGS = -O2 -funroll-loops -ffast-math -fomit-frame-pointer -finline-functions -s

ifeq ($(findstring "g++",$(CXX)),"g++")
WARNINGS += -Wlogical-op
RELEASE_FLAGS += -fno-strength-reduce
endif

BUILD_FLAGS = $(RELEASE_FLAGS)

debug: BUILD_FLAGS:=$(DEBUG_FLAGS)

ifndef DEBUG
ifeq ($(LAST_BUILD_IN_DEBUG), 1)
FORCED_DEBUG = 1
DEBUG = 1
endif
endif

ifndef WITHOUT_GL
COMMON_CFLAGS += -DWITH_GL
endif

ifdef DEBUG
BUILD_FLAGS = $(DEBUG_FLAGS)
all: check_deps debug
else
all: check_deps distrib
endif

# gtest doesn't build with warnings flags, hence the COMMON_CFLAGS
ALL_CFLAGS=$(COMMON_CFLAGS) $(WARNINGS)

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
	@sdl2-config --cflags >/dev/null 2>&1 || (echo "Error: missing dependency libSDL2. Try installing libsdl2 development package (e.g: libsdl2-dev)" && false)
	@pkg-config --version >/dev/null 2>&1 || (echo "Error: missing pkg-config. Try installing pkg-config" && false)
	@pkg-config --cflags freetype2 >/dev/null 2>&1 || (echo "Error: missing dependency libfreetype. Try installing libfreetype development package (e.g: libfreetype6-dev)" && false)
	@pkg-config --cflags zlib >/dev/null 2>&1 || (echo "Error: missing dependency zlib. Try installing zlib development package (e.g: zlib1g-dev)" && false)
	@pkg-config --cflags libpng >/dev/null 2>&1 || (echo "Error: missing dependency libpng. Try installing libpng development package (e.g: libpng-dev)" && false)
else
# TODO(cpitrat): Implement check_deps for windows build
check_deps:
endif

tags:
	@ctags -R main.cpp src || echo -e "!!!!!!!!!!!\n!! Warning: ctags not found - if you are a developer, you might want to install it.\n!!!!!!!!!!!"

doc: $(HTML_DOC)

$(HTML_DOC): $(GROFF_DOC)
	groff -mandoc -Thtml $< > $@

cap32.cfg: cap32.cfg.tmpl
	@sed 's/__SHARE_PATH__.*//' cap32.cfg.tmpl > cap32.cfg

$(TARGET): $(OBJECTS) $(MAIN) cap32.cfg
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN) $(LIBS)

ifeq ($(PLATFORM),windows)
DLLS = SDL2.dll libbz2-1.dll libfreetype-6.dll libpng16-16.dll libstdc++-6.dll \
       libwinpthread-1.dll zlib1.dll libglib-2.0-0.dll libgraphite2.dll \
       libharfbuzz-0.dll libiconv-2.dll libintl-8.dll libpcre2-8-0.dll \
			 libbrotlidec.dll libbrotlicommon.dll

distrib: $(TARGET)
	mkdir -p $(ARCHIVE_DIR)
	rm -f $(RELEASE_DIR)/$(ARCHIVE).zip
	cp $(TARGET) $(ARCHIVE_DIR)/
	$(foreach DLL,$(DLLS),[ -f $(MINGW_PATH)/bin/$(DLL) ] && cp $(MINGW_PATH)/bin/$(DLL) $(ARCHIVE_DIR)/ || (echo "$(MINGW_PATH)/bin/$(DLL) doesn't exist" && false);)
	cp $(MINGW_PATH)/bin/libgcc_s_*-1.dll $(ARCHIVE_DIR)/
	cp cap32.cfg.tmpl cap32.cfg COPYING.txt README.md $(ARCHIVE_DIR)/
	cp -r resources/ rom/ licenses/ $(ARCHIVE_DIR)/
	cd $(RELEASE_DIR) && zip -r $(ARCHIVE).zip $(ARCHIVE)

install: $(TARGET)

else

ifeq ($(ARCH),macos)

# Create a zip with a cap32 binary that should work launched locally
distrib: $(TARGET)
	mkdir -p $(ARCHIVE_DIR)
	rm -f $(RELEASE_DIR)/$(ARCHIVE).zip
	cp $(TARGET) $(ARCHIVE_DIR)/
	cp -r rom resources doc licenses $(ARCHIVE_DIR)
	cp cap32.cfg README.md COPYING.txt $(ARCHIVE_DIR)
	cd $(RELEASE_DIR) && zip -r $(ARCHIVE).zip $(ARCHIVE)

else

SRC_PACKAGE_DIR=$(ARCHIVE_DIR)/caprice32-$(VERSION)

# Create a debian source package
distrib: $(TARGET)
	mkdir -p $(SRC_PACKAGE_DIR)
	rm -fr $(SRC_PACKAGE_DIR)/*
	cp -r src rom resources doc licenses debian $(SRC_PACKAGE_DIR)
	cp main.cpp cap32.cfg.tmpl cap32.cfg makefile README.md INSTALL.md COPYING.txt $(SRC_PACKAGE_DIR)
	tar jcf $(SRC_PACKAGE_DIR).tar.bz2 -C $(ARCHIVE_DIR) caprice32-$(VERSION)
	ln -s caprice32-$(VERSION).tar.bz2 $(ARCHIVE_DIR)/caprice32_$(VERSION).orig.tar.bz2 || true

endif  # ARCH =? macos

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)$(prefix)/bin/$(TARGET)
	install -D $(GROFF_DOC) $(DESTDIR)$(prefix)/share/man/man6/cap32.6
	if [ ! -f $(DESTDIR)/etc/cap32.cfg ]; then \
		install -D -m664 cap32.cfg.tmpl $(DESTDIR)/etc/cap32.cfg; \
		sed -i "s,__SHARE_PATH__,$(DESTDIR)$(prefix)/share/caprice32," $(DESTDIR)/etc/cap32.cfg; \
	fi
	mkdir -p $(DESTDIR)$(prefix)/share/caprice32
	cp -r resources rom $(DESTDIR)$(prefix)/share/caprice32

endif

####################################
### Tests
####################################

googletest:
	@[ -d googletest ] || git clone https://github.com/google/googletest.git

TEST_CFLAGS = $(COMMON_CFLAGS) -I$(GTEST_DIR)/include -I$(GTEST_DIR) -I$(GMOCK_DIR)/include -I$(GMOCK_DIR)
GTEST_DIR = googletest/googletest/
GMOCK_DIR = googletest/googlemock/

$(GTEST_DIR)/src/gtest-all.cc: googletest
$(GMOCK_DIR)/src/gmock-all.cc: googletest

$(TEST_DEPENDS): $(OBJDIR)/%.d: %.cpp
	@echo Computing dependencies for $<
	@mkdir -p `dirname $@`
	@$(CXX) -MM $(BUILD_FLAGS) $(TEST_CFLAGS) $< | { sed 's#^[^:]*\.o[ :]*#$(OBJDIR)/$*.o $(OBJDIR)/$*.d : #g' ; echo "%.h:;" ; echo "" ; } > $@

$(TEST_OBJECTS): $(OBJDIR)/%.o: %.cpp googletest
	$(CXX) -c $(BUILD_FLAGS) $(TEST_CFLAGS) -o $@ $<

$(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc googletest
	@mkdir -p `dirname $@`
	$(CXX) -c $(BUILD_FLAGS) $(TEST_CFLAGS) -o $@ $<

$(OBJDIR)/$(GMOCK_DIR)/src/gmock-all.o: $(GMOCK_DIR)/src/gmock-all.cc googletest
	@mkdir -p `dirname $@`
	$(CXX) -c $(BUILD_FLAGS) $(TEST_CFLAGS) -o $@ $<

$(TEST_TARGET): $(OBJECTS) $(TEST_OBJECTS) $(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o $(OBJDIR)/$(GMOCK_DIR)/src/gmock-all.o
	$(CXX) $(LDFLAGS) -o $(TEST_TARGET) $(OBJDIR)/$(GTEST_DIR)/src/gtest-all.o $(OBJDIR)/$(GMOCK_DIR)/src/gmock-all.o $(TEST_OBJECTS) $(OBJECTS) $(LIBS) -lpthread

ifeq ($(PLATFORM),windows)
unit_test: $(TEST_TARGET) distrib
	cp $(TEST_TARGET) $(ARCHIVE_DIR)/
	rm -fr $(ARCHIVE_DIR)/test
	ln -s -f ../../test $(ARCHIVE_DIR)/test
	cd $(ARCHIVE_DIR) && ./$(TEST_TARGET) --gtest_shuffle

e2e_test: $(TARGET)
	cd test/integrated && ./run_tests.sh
else
unit_test: $(TEST_TARGET)
	./$(TEST_TARGET) --gtest_shuffle

e2e_test: $(TARGET)
	cd test/integrated && ./run_tests.sh
endif

deb_pkg: all
	# Both changelog files need to be patched with the proper version !
	sed -i "1s/(.*)/($(VERSION)-$(REVISION))/" debian/changelog
	sed -i "1s/(.*)/($(VERSION)-$(REVISION))/" $(ARCHIVE_DIR)/caprice32-$(VERSION)/debian/changelog
	cd $(ARCHIVE_DIR)/caprice32-$(VERSION)/debian && debuild -e CXX -us -uc --lintian-opts --profile debian

BUNDLE_DIR=release/cap32-macos-bundle/Caprice32.app
macos_bundle: all
	rm -rf $(BUNDLE_DIR)
	mkdir -p $(BUNDLE_DIR)/Contents/MacOS
	mkdir -p $(BUNDLE_DIR)/Contents/Resources
	install $(TARGET) $(BUNDLE_DIR)/Contents/MacOS/Caprice32
	install resources/Info.plist $(BUNDLE_DIR)/Contents/
	install -m664 cap32.cfg.tmpl $(BUNDLE_DIR)/Contents/Resources/cap32.cfg
	gsed -i "s,__SHARE_PATH__,../Resources," $(BUNDLE_DIR)/Contents/Resources/cap32.cfg
	cp -r resources rom $(BUNDLE_DIR)/Contents/Resources
	mkdir -p $(BUNDLE_DIR)/Contents/Frameworks
	# Copy shared libs. For some reason, it can happen that some are not found. Not sure why, let's just ignore it.
	for lib in `otool -L $(BUNDLE_DIR)/Contents/MacOS/Caprice32 | grep ".dylib" | awk '{ print $$1 }'`; do echo "Copying '$$lib'"; cp "$$lib" $(BUNDLE_DIR)/Contents/Frameworks/; done || true
	# Retry hdiutil up to 3 times: it occasionally fails with "Resource Busy"
	for i in 1 2 3; do hdiutil create -volname Caprice32-$(VERSION) -srcfolder $(BUNDLE_DIR) -ov -format UDZO release/cap32-macos-bundle/Caprice32.dmg && break || sleep 5; done

clang-tidy:
	if $(CLANG_TIDY) -checks=-*,$(CLANG_CHECKS) $(SOURCES) -header-filter=src/* -- $(COMMON_CFLAGS) | grep "."; then false; fi
	./tools/check_includes.sh

clang-format:
	./tools/check_clang_format.sh $(CLANG_FORMAT) "-style=Google" $(SOURCES) $(TEST_SOURCES) $(HEADERS) $(TEST_HEADERS)

fix-clang-format:
	$(CLANG_FORMAT) -style=Google -i $(SOURCES) $(TEST_SOURCES) $(HEADERS) $(TEST_HEADERS)

doxygen:
	doxygen doxygen.cfg

clean:
	rm -rf obj/ release/ .pc/ doxygen/
	rm -f test_runner test_runner.exe cap32 cap32.exe .debug tags

-include $(DEPENDS) $(TEST_DEPENDS)
