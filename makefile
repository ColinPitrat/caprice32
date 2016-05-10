# makefile for compiling native unix builds
# use "make DEBUG=TRUE" to build a debug executable

LAST_BUILD_IN_DEBUG=$(shell [ -e .debug ] && echo 1 || echo 0)
# Search these directories for source files:
VPATH = gui/src:gui/includes

.SUFFIXES : .o .cpp

ifndef CXX
CXX	= g++
endif

GFLAGS	= -std=c++11 -Wall `sdl-config --cflags`

ifeq ($(LAST_BUILD_IN_DEBUG), 1)
FORCED_DEBUG=1
DEBUG=1
endif

ifdef DEBUG
DFLAGS	= $(GFLAGS) -g -Og
all: debug
else
DFLAGS	= $(GFLAGS) -O2 -funroll-loops -ffast-math -fomit-frame-pointer -fno-strength-reduce -finline-functions -s
all: cap32
endif

ifdef WITHOUT_GL
CFLAGS = $(DFLAGS)
else
CFLAGS = $(DFLAGS) -DHAVE_GL
endif

IPATHS	= -I. -Igui/includes `freetype-config --cflags` 

LIBS = `sdl-config --libs` -lz `freetype-config --libs` 

# wGui and 'own' gui components:
GUIOBJS = CapriceLoadSave.o CapriceAbout.o CapriceGui.o CapriceGuiView.o CapriceOptions.o CapriceRomSlots.o \
CapriceMemoryTool.o std_ex.o wg_application.o wg_button.o wg_checkbox.o wg_radiobutton.o wg_menu.o wg_color.o \
wg_dropdown.o wg_editbox.o wg_fontengine.o \
wg_frame.o wg_groupbox.o wg_label.o wg_listbox.o wg_messagebox.o wg_message_client.o wg_message.o \
wg_message_server.o wg_navigationbar.o wg_painter.o wg_picture.o wg_point.o \
wg_progress.o wg_rect.o wg_renderedstring.o wg_resource_handle.o wg_resources.o \
wg_scrollbar.o wg_tab.o wg_textbox.o wg_timer.o wg_toolbar.o wg_tooltip.o \
wg_view.o wg_window.o wutil_config_store.o wutil_log.o

.cpp.o :
	$(CXX) -c $(CFLAGS) $(IPATHS) -o $@ $<

debug: debug_flag cap32

debug_flag:
ifdef FORCED_DEBUG
	@echo -e '\n!!!!!!!!!!!\n!! Warning: previous build was in debug - rebuilding in debug.\n!! Use make clean before running make to rebuild in release.\n!!!!!!!!!!!\n'
endif
	@touch .debug

cap32: cap32.cpp $(GUIOBJS) crtc.o fdc.o glfuncs.o psg.o tape.o video.o z80.o fileutils.o cap32.h z80.h
	$(CXX) $(CFLAGS) $(IPATHS) -o cap32 cap32.cpp crtc.o fdc.o glfuncs.o psg.o tape.o video.o z80.o fileutils.o $(GUIOBJS) $(LIBS)

crtc.o: crtc.c cap32.h crtc.h z80.h
	$(CXX) $(CFLAGS) $(IPATHS) -c crtc.c

fdc.o: fdc.c cap32.h z80.h
	$(CXX) $(CFLAGS) $(IPATHS) -c fdc.c

glfuncs.o: glfuncs.cpp glfuncs.h glfunclist.h
	$(CXX) $(CFLAGS) $(IPATHS) -c glfuncs.cpp

psg.o: psg.c cap32.h z80.h
	$(CXX) $(CFLAGS) $(IPATHS) -c psg.c

tape.o: tape.c cap32.h tape.h z80.h
	$(CXX) $(CFLAGS) $(IPATHS) -c tape.c

video.o: video.cpp video.h glfuncs.h glfunclist.h cap32.h
	$(CXX) $(CFLAGS) $(IPATHS) -c video.cpp

z80.o: z80.c z80.h cap32.h tape.h z80daa.h
	$(CXX) $(CFLAGS) $(IPATHS) -c z80.c

fileutils.o: fileutils.cpp fileutils.h

####################################
### Dependencies related to the gui:
####################################

cap32.o: cap32.cpp cap32.h crtc.h tape.h video.h z80.h CapriceGui.h CapriceGuiView.h font.c rom_mods.c

CapriceMemoryTool.o: CapriceMemoryTool.cpp CapriceMemoryTool.h cap32.h std_ex.h CapriceRomSlots.h wgui.h \
 wg_checkbox.h wg_frame.h wg_groupbox.h wg_label.h wg_navigationbar.h wg_radiobutton.h wg_scrollbar.h

CapriceLoadSave.o: CapriceLoadSave.cpp CapriceLoadSave.h cap32.h std_ex.h CapriceRomSlots.h wgui.h \
 wg_checkbox.h wg_frame.h wg_groupbox.h wg_label.h wg_navigationbar.h wg_radiobutton.h wg_scrollbar.h

CapriceAbout.o: CapriceAbout.cpp CapriceAbout.h wg_label.h wg_fontengine.h wg_messagebox.h wg_application.h cap32.h

CapriceGui.o: CapriceGui.cpp CapriceGui.h wg_application.h

CapriceGuiView.o: CapriceGuiView.cpp CapriceGuiView.h CapriceLoadSave.h CapriceMemoryTool.h CapriceAbout.h CapriceOptions.h wg_view.h wg_application.h cap32.h

CapriceOptions.o: CapriceOptions.cpp CapriceOptions.h cap32.h std_ex.h CapriceRomSlots.h wgui.h \
 wg_checkbox.h wg_frame.h wg_groupbox.h wg_label.h wg_navigationbar.h wg_radiobutton.h wg_scrollbar.h

CapriceRomSlots.o: CapriceRomSlots.cpp CapriceRomSlots.h cap32.h wg_frame.h wg_label.h wgui.h fileutils.h

std_ex.o: std_ex.cpp std_ex.h

wg_application.o: wg_application.cpp wg_application.h std_ex.h wg_error.h wg_fontengine.h \
 wg_message_client.h wg_message_server.h wg_resources.h wg_window.h wgui_include_config.h \
 wutil_config_store.h wutil_log.h wutil_debug.h cap32.h

wg_button.o: wg_button.cpp wg_button.h std_ex.h wg_application.h wg_message_server.h \
 wg_painter.h wg_picture.h wg_renderedstring.h wg_window.h wgui_include_config.h wutil_debug.h

wg_checkbox.o: wg_checkbox.cpp wg_checkbox.h wg_message_server.h wg_painter.h wg_resources.h \
 wg_window.h wgui_include_config.h

wg_color.o: wg_color.cpp wg_color.h std_ex.h wgui_include_config.h

wg_dropdown.o: wg_dropdown.cpp wg_dropdown.h std_ex.h wg_editbox.h wg_listbox.h \
 wg_message_server.h wg_painter.h wg_resources.h wg_view.h wg_window.h wgui_include_config.h

wg_editbox.o: wg_editbox.cpp wg_editbox.h std_ex.h  wg_application.h wg_error.h wg_message_server.h \
 wg_painter.h wg_renderedstring.h wg_resources.h wg_timer.h wg_view.h wg_window.h wgui_include_config.h \
 wutil_debug.h

wg_fontengine.o: wg_fontengine.cpp wg_fontengine.h wg_application.h wg_error.h wgui_include_config.h

wg_frame.o: wg_frame.cpp wg_frame.h wg_application.h wg_button.h wg_view.h \
 wg_window.h wgui_include_config.h

wg_groupbox.o: wg_groupbox.cpp wg_groupbox.h wg_application.h wg_painter.h wg_renderedstring.h \
 wg_window.h wgui_include_config.h

wg_label.o: wg_label.cpp wg_label.h wg_application.h wg_painter.h wg_renderedstring.h \
  wg_window.h wgui_include_config.h

wg_listbox.o: wg_listbox.cpp wg_listbox.h std_ex.h wg_application.h wg_error.h wg_message_server.h \
 wg_painter.h wg_renderedstring.h wg_scrollbar.h wg_window.h wgui_include_config.h wutil_debug.h

wg_menu.o: wg_menu.cpp wg_menu.h wg_application.h wg_error.h wg_message_server.h wg_painter.h \
 wg_renderedstring.h wg_resources.h wg_timer.h wg_view.h wg_window.h wgui_include_config.h \
 wutil_debug.h std_ex.h

wg_message.o: wg_message.cpp wg_message.h std_ex.h wg_point.h wg_rect.h wgui_include_config.h wutil_debug.h

wg_message_client.o: wg_message_client.cpp wg_message_client.h wg_error.h wg_message.h \
 wg_message_server.h wgui_include_config.h

wg_message_server.o: wg_message_server.cpp wg_message_server.h std_ex.h wg_application.h \
 wg_error.h wg_message.h wg_message_client.h wgui_include_config.h wutil_debug.h

wg_messagebox.o: wg_messagebox.cpp wg_messagebox.h wg_frame.h wg_label.h wgui_include_config.h

wg_navigationbar.o: wg_navigationbar.cpp wg_navigationbar.h std_ex.h wg_application.h \
  wg_error.h wg_message_server.h wg_painter.h wg_renderedstring.h wg_resource_handle.h \
  wg_window.h wgui_include_config.h wutil_debug.h

wg_painter.o: wg_painter.cpp std_ex.h wg_color.h wg_error.h \
  wg_painter.h wg_point.h wg_rect.h wg_window.h wgui_include_config.h

wg_picture.o: wg_picture.cpp wg_picture.h wgui_include_config.h wg_window.h wg_painter.h wg_resource_handle.h

wg_point.o: wg_point.cpp wg_point.h std_ex.h wgui_include_config.h

wg_progress.o: wg_progress.cpp wg_progress.h wgui_include_config.h wg_painter.h wg_range_control.h \
  std_ex.h

wg_radiobutton.o: wg_radiobutton.cpp wg_radiobutton.h wg_window.h wg_painter.h wg_resources.h \
  wg_message_server.h wgui_include_config.h

wg_rect.o: wg_rect.cpp wg_rect.h wgui_include_config.h wg_error.h std_ex.h wg_point.h

wg_renderedstring.o: wg_renderedstring.cpp wg_renderedstring.h wgui_include_config.h wg_error.h \
  wg_painter.h wg_fontengine.h wg_rect.h std_ex.h wg_color.h

wg_resource_handle.o: wg_resource_handle.cpp wg_resource_handle.h wgui_include_config.h wg_error.h \
  wutil_debug.h

wg_resources.o: wg_resources.cpp wg_resources.h wgui_include_config.h wg_error.h \
  wg_painter.h wutil_debug.h wg_application.h wg_resource_handle.h wg_color.h

wg_scrollbar.o: wg_scrollbar.cpp wg_scrollbar.h wg_range_control.h wg_painter.h wg_button.h \
  wgui_include_config.h wg_application.h wg_message_server.h wg_error.h wg_resources.h std_ex.h

wg_textbox.o: wg_textbox.cpp wg_textbox.h wg_application.h wg_error.h wg_message_server.h wg_painter.h \
  wg_renderedstring.h wg_resources.h wg_scrollbar.h wg_timer.h wg_view.h wg_window.h wgui_include_config.h \
  wutil_debug.h std_ex.h \

wg_timer.o: wg_timer.cpp wg_timer.h wgui_include_config.h wg_message_server.h wg_message_client.h

wg_toolbar.o: wg_toolbar.cpp wg_toolbar.h wg_button.h wg_window.h wgui_include_config.h \
  wg_message_server.h wutil_debug.h

wg_tooltip.o: wg_tooltip.cpp wg_tooltip.h wg_application.h wg_painter.h wg_renderedstring.h \
  wg_timer.h wg_view.h wg_window.h wgui_include_config.h

wg_view.o: wg_view.cpp wg_view.h wg_application.h wg_error.h wg_frame.h wg_menu.h wg_message_server.h \
  wg_painter.h wg_window.h wgui_include_config.h wutil_debug.h std_ex.h

wg_window.o: wg_window.cpp wg_window.h wg_application.h wg_color.h wg_error.h wg_message_client.h \
  wg_message_server.h wg_painter.h wg_rect.h wg_view.h wgui_include_config.h wutil_debug.h \
  std_ex.h

wutil_config_store.o: wutil_config_store.cpp wutil_config_store.h std_ex.h wgui_include_config.h \
  wutil_debug.h

wutil_log.o: wutil_log.cpp wutil_log.h wgui_include_config.h

wg_tab.o: wg_tab.cpp wg_tab.h

clean:
	rm -f *.o cap32 .debug
