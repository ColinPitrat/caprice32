![Caprice32 logo](https://raw.githubusercontent.com/ColinPitrat/caprice32/master/resources/cap32logo.bmp)
# Caprice32 - Amstrad CPC Emulator

(c) Copyright 1997-2015 Ulrich Doewich

(c) Copyright 2016-2019 Colin Pitrat

https://github.com/ColinPitrat/caprice32

Linux build: [![Linux build Status](https://travis-ci.org/ColinPitrat/caprice32.svg?branch=master)](https://travis-ci.org/ColinPitrat/caprice32)

 Windows build: [![Windows build status](https://ci.appveyor.com/api/projects/status/h795wt2xlttaj6sk?svg=true)](https://ci.appveyor.com/project/ColinPitrat/caprice32)


# What is it ?

Caprice32 is a software emulator of the Amstrad CPC 8bit home computer series running on Linux and Windows. The emulator faithfully imitates the CPC464, CPC664, and CPC6128 models. By recreating the operations of all hardware components at a low level, the emulator achieves a high degree of compatibility with original CPC software. These programs or games can be run unmodified at real-time or higher speeds, depending on the emulator host environment.

# Features

Caprice32 provides:
  * Complete emulation of CPC464, CPC664 and CPC6128
  * Mostly working support of Plus Range: CPC464+/CPC6128+/GX4000 (missing vectored & DMA interrupts, analog joysticks and 8 bit printer)
  * Joystick support - it can be fully used with joystick only, thanks to an integrated virtual keyboard.
  * Joystick emulation - joystick-only games can be played using the keyboard
  * English, French or Spanish keyboards
  * DSK and [IPF](http://softpres.org/glossary:ipf) files for disks - VOC and CDT files for tapes - CPR files for cartridge
  * Snapshots (SNA files)
  * Direct load of ZIP files
  * Custom disk formats
  * Printer support
  * Memory tool to inspect and modify memory (peek and poke)
  * Experimental support of Multiface 2 (you should prefer using memory tool)
  * Text mode graphics (using [aalib](http://aa-project.sourceforge.net/aalib/) or [libcaca](http://caca.zoy.org/wiki/libcaca))

You see something missing ? Do not hesitate to open an issue to suggest it.

# Basic usage

See the [manual page](http://htmlpreview.github.io/?https://github.com/ColinPitrat/caprice32/blob/master/doc/man.html) for more details. If you are really lost, you can simply invoke the emulator without any argument, then press F1 to get the in-emulator menu.

# Help needed

Maintaining Caprice is a lot of work and you can help with it.
You can:
  * Use it, show it, talk and write about it
  * Thank the maintainer
  * Report any bug or missing feature
  * Write some documentation
  * Package it for your favourite distribution (if not yet available)
  * Port it to iOS so that Mac users can enjoy it too

# Supporting

You can support me on Liberapay:

[<img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg">](https://liberapay.com/ColinPitrat/donate)

# Building and compiling

See the [INSTALL.md](INSTALL.md) files for Caprice32 build instructions.

# Using the source

The source for Caprice32 is distributed under the terms of the GNU General Public License version 2 (GPLv2), which is included in this archive as COPYING.txt. Please make sure that you understand the terms and conditions of the license before using the source.

The screen dump part of Caprice32 uses [driedfruit SDL_SavePNG](https://github.com/driedfruit/SDL_SavePNG) code, released under zlib/libpng license, which is compatible with GPLv2.

I encourage you to get involved in the project.

# Comments or ready to contribute?

If you have suggestions, a bug report or even want to participate to the development, please feel free to open an issue or submit a pull request.

# Why another GitHub repository ?

There are many repositories for caprice32 on GitHub:

  * https://github.com/burzumishi/caprice32
  * https://github.com/rofl0r/caprice32
  * https://github.com/MrZammler/caprice32
  * https://github.com/egrath/caprice32-mod
  * https://github.com/burzumishi/caprice32wx

So why create another one ? All these repositories are highly inactive. The ones that touched the code added dependencies (wxWidget, GTK) without really adding features.
