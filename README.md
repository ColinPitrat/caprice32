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

See the manual page for invocation arguments, key mapping, and other details. If you are really lost, you can simply invoke the emulator without any argument, then press F1 to get the in-emulator menu.

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
  * libPNG (optional) - http://libpng.org/pub/png/libpng.html

# Compiling

#### Linux target:

To build with default options, use :

`make`

The following options are available:

 * `DEBUG=TRUE`
 * `WITHOUT_GL=TRUE`
 * `WITHOUT_PNG=TRUE`

For example, for a debug build, use:

`make DEBUG=TRUE`

#### Debian/Ubuntu package:

To build a debian package on Debian/Ubuntu distributions install the dependencies as mentioned above and the dpkg-dev package:

`sudo apt-get install dpkg-dev`

Then build the package with the following command:

`dpkg-buildpackage -rfakeroot -uc -us`

After installation of the package the users should copy the /etc/cap32.cfg file to their homedirectory with the following command:

`cp /etc/cap32.cfg .cap32.cfg`

#### Windows target:

Edit the makefile to update the `MINGW_PATH`, `WINCXX`, `WININCS` and `WINLIBS` directive with the correct location of the SDL include and library files.
To build with default options, use:

`make windows`

# Using the source

The source for Caprice32 is distributed under the terms of the GNU General Public License version 2 (GPLv2), which is included in this archive as COPYING.txt. Please make sure that you understand the terms and conditions of the license before using the source.
The screen dump part of Caprice32 uses [driedfruit SDL_SavePNG] (https://github.com/driedfruit/SDL_SavePNG) code, released under zlib/libpng license, which is compatible with GPLv2.
I encourage you to get involved in the project.

# Comments or ready to contribute?

If you have suggestions, a bug report or even want to participate to the development, please feel free to open an issue or submit a pull request.

# Why another GitHub repository ?

There are many repositories for caprice32 on GitHub:

  * https://github.com/burzumishi/caprice32
  * https://github.com/rofl0r/caprice32
  * https://github.com/Neophile76/Caprice32
  * https://github.com/MrZammler/caprice32
  * https://github.com/egrath/caprice32-mod
  * https://github.com/burzumishi/caprice32wx

So why create another one ? All these repositories are highly inactive but more than that, in my opinion, they took a wrong direction. Some added dependencies (wxWidget, GTK) without really adding features. Two imported the code but did not do any update.
