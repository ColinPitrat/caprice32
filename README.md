Caprice32 - Amstrad CPC Emulator

(c) Copyright 1997-2004 Ulrich Doewich

https://github.com/ColinPitrat/caprice32

# What is it ?

Caprice32 is a software emulator of the Amstrad CPC 8bit home computer series. The emulator faithfully imitates the CPC464, CPC664, and CPC6128 models. By recreating the operations of all hardware components at a low level, the emulator achieves a high degree of compatibility with original CPC software. These programs or games can be run unmodified at real-time or higher speeds, depending on the emulator host environment.

# Using the source

The source for Caprice32 is distributed under the terms of the GNU General Public License (GNU GPL), which is included in this archive as COPYING.txt. Please make sure that you understand the terms and conditions of the license before using the source.

I encourage you to get involved in the project.

# Requirements

You will need the following to successfully compile an executable:

  * MinGW (only for Windows) - http://sourceforge.net/projects/mingw/
  * SDL - http://www.libsdl.org/index.php
  * zLib - http://www.gzip.org/zlib/

# Compiling

#### Linux target:

Use :

`make` 

to compile a developer build, or use: 

`make RELEASE=TRUE` 

for an optimized executable.

#### Windows target:

Edit the makefile to update the IPATHS directive with the correct location of the SDL include and library files.
Use:

`make -f makefile.windows` (or `mingw32-make -f makefile.windows`, depending on how you have your MinGW installed) 

to compile a developer build, or use: 

`make -f makefile.windows RELEASE=TRUE` 

for an optimized executable.

# Comments or ready to contribute?

If you have suggestions, a bug report or even want to join the development team, please feel free to use one of the many contact methods presented on the Caprice32 project page on SourceForge.net - see the URL at the top.

# Why another GitHub repository ?

There are many repository for caprice32 on GitHub:

  * https://github.com/sebhz/caprice32
  * https://github.com/burzumishi/caprice32
  * https://github.com/rofl0r/caprice32
  * https://github.com/Neophile76/Caprice32
  * https://github.com/MrZammler/caprice32
  * https://github.com/egrath/caprice32-mod
  * https://github.com/burzumishi/caprice32wx

So why create another one ? All these repository are highly inactive but more than that, in my opinion, they took a wrong direction. Some added dependencies (wxWidget, GTK) without really adding feature. Two imported the code but do not did not do any update. The most promising one in my opinion is sebhz's one but the addition in it of some games images annoys me too much to contribute to it !
