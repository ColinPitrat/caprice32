![Caprice32 logo](https://raw.githubusercontent.com/ColinPitrat/caprice32/master/resources/cap32logo.bmp)
# Caprice32 - Amstrad CPC Emulator

(c) Copyright 1997-2015 Ulrich Doewich

(c) Copyright 2016 Colin Pitrat

https://github.com/ColinPitrat/caprice32

[![Build Status](https://travis-ci.org/ColinPitrat/caprice32.svg?branch=master)](https://travis-ci.org/ColinPitrat/caprice32)

# What is it ?

Caprice32 is a software emulator of the Amstrad CPC 8bit home computer series running on Linux and Windows. The emulator faithfully imitates the CPC464, CPC664, and CPC6128 models. By recreating the operations of all hardware components at a low level, the emulator achieves a high degree of compatibility with original CPC software. These programs or games can be run unmodified at real-time or higher speeds, depending on the emulator host environment.

# Features

Caprice32 provides:
  * Complete emulation of CPC464, CPC664 and CPC6128
  * Mostly working support of CPC6128+ (missing vectored & DMA interrupts, analog joysticks and 8 bit printer)
  * Joystick support - it can be fully used with joystick only, thanks to an integrated virtual keyboard.
  * Joystick emulation - joystick-only games can be played using the keyboard
  * English, French or Spanish keyboards
  * DSK files for disks - VOC and CDT files for tapes - CPR files for cartridge
  * Direct load of ZIP files
  * Custom disk formats
  * Snapshots
  * Printer support
  * Memory tool to inspect and modify memory (peek and poke)
  * Experimental support of Multiface 2 (you should prefer using memory tool)
  * Text mode graphics (using [aalib](http://aa-project.sourceforge.net/aalib/) or [libcaca](http://caca.zoy.org/wiki/libcaca))

You see something missing ? Do not hesitate to open an issue to suggest it.

# Basic usage

## Controls

Emulator functionalities are available through function keys (F1-F12).
As CPC also had function keys, they are emulated by keys from the numpad (which makes sense because their disposition on the CPC was similar to the numpad).
If your computer doesn't have a numpad (e.g laptop that doesn't support it with Fn key) the only way to access the CPC F1-F10 keys is through the virtual keyboard.

F1 - Show GUI (and pause the emulator)
F2 - Toggle fullscreen / windowed mode
F4 - Press play for tape
F5 - Reset
F6 - Multiface II Stop (advanced users only)
F7 - Toggle joystick emulation (when active, keyboard arrows, Z and X are remapped to emulate joystick 1 of the CPC - real joysticks are disabled)
F8 - Toggle display of FPS
F9 - Toggle limitation of speed (you may want it when loading from a tape ! or for benchmarking)
F10 - Quit
F12 - Toggle debug mode (more verbose with debug build)

Shift + F1 - Show Virtual Keyboard (useful if you have no keyboard or don't find the key you want to press on it)

## Loading a media (disk/tape/cartridge...)

Simplest solution is to launch the emulator specifying the image you want to use:

`cap32 /path/to/mydisk.dsk`

In a desktop environment, you can usually achieve the same result by drag & dropping the icon of the file on the icon of the emulator.


You can also load a media in the emulator while it is running through the menu. Press F1 to show the GUI and select 'Load/Save'.
Choose the file type you want to load and select it from the box below.
You can configure the default directory for each type of media from the configuration file.

## Configuration file

When starting, cap32 is looking for a configuration file in the following directories (taking the first one that matches):
  * $CWD/cap32.cfg ($CWD being the directory where cap32 is situated)
  * $HOME/.cap32.cfg ($HOME being the user's home directory)
  * /etc/cap32.cfg

The file contain various configuration parameters, some of which can be modified from the GUI.
When saving the configuration from the GUI, it will be written in $CWD/cap32.cfg if it exists, otherwise in $HOME/.cap32.cfg.

# Help needed

Maintaining Caprice is a lot of work and you can help with it.
You can:
  * Use it, show it, talk and write about it
  * Thank the maintainer
  * Report any bug or missing feature
  * Write some documentation
  * Help building and testing the windows version
  * Port it to iOS so that Mac users can enjoy it too

# Requirements

You will need the following to successfully compile an executable:

  * MinGW (only for Windows) - http://sourceforge.net/projects/mingw/
  * SDL - http://www.libsdl.org/index.php
  * FreeType - https://www.freetype.org/
  * zLib - http://www.gzip.org/zlib/

# Compiling

#### Linux target:

To build with default options, use :

`make`

The following options are available:

 * `DEBUG=TRUE`
 * `WITHOUT_GL=TRUE`

For example, for a debug build, use:

`make DEBUG=TRUE`

#### Debian/Ubuntu package:

To build a debian package on Debian/Ubuntu distributions install the dependencies as mentioned above and the dpkg-dev package:

`sudo install dpkg-dev`

Then build the package with the following command:

`dpkg-buildpackage -rfakeroot -uc -us`

After installation of the package the users should copy the /etc/cap32.cfg file to their homedirectory with the following command:

`cp /etc/cap32.cfg .cap32.cfg`

#### Windows target:

Edit the makefile to update the `MINGW_PATH`, `WINCXX`, `WININCS` and `WINLIBS` directive with the correct location of the SDL include and library files.
To build with default options, use:

`make windows`

# Using the source

The source for Caprice32 is distributed under the terms of the GNU General Public License (GNU GPL), which is included in this archive as COPYING.txt. Please make sure that you understand the terms and conditions of the license before using the source.

I encourage you to get involved in the project.

# Comments or ready to contribute?

If you have suggestions, a bug report or even want to participate to the development, please feel free to open an issue or submit a pull request.

# Why another GitHub repository ?

There are many repository for caprice32 on GitHub:

  * https://github.com/burzumishi/caprice32
  * https://github.com/rofl0r/caprice32
  * https://github.com/Neophile76/Caprice32
  * https://github.com/MrZammler/caprice32
  * https://github.com/egrath/caprice32-mod
  * https://github.com/burzumishi/caprice32wx

So why create another one ? All these repositories are highly inactive but more than that, in my opinion, they took a wrong direction. Some added dependencies (wxWidget, GTK) without really adding feature. Two imported the code but did not do any update.
