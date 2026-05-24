![Caprice32 logo](https://raw.githubusercontent.com/ColinPitrat/caprice32/master/resources/cap32logo.bmp)
# Caprice32 - Amstrad CPC Emulator

(c) Copyright 1997-2015 Ulrich Doewich

(c) Copyright 2016-2025 Colin Pitrat

https://github.com/ColinPitrat/caprice32

Linux build: [![Linux build Status](https://github.com/ColinPitrat/caprice32/actions/workflows/linux.yml/badge.svg?branch=master)](https://github.com/ColinPitrat/caprice32/actions/workflows/linux.yml)

Windows build: [![Windows build Status](https://github.com/ColinPitrat/caprice32/actions/workflows/windows.yml/badge.svg?branch=latest)](https://github.com/ColinPitrat/caprice32/actions/workflows/windows.yml)

MacOS build: [![MacOS build status](https://github.com/ColinPitrat/caprice32/actions/workflows/macos.yml/badge.svg?branch=latest)](https://github.com/ColinPitrat/caprice32/actions/workflows/macos.yml)

# What is it ?

Caprice32 is a software emulator of the Amstrad CPC 8-bit home computer series running on Linux, macOS and Windows. The emulator faithfully imitates the CPC464, CPC664, and CPC6128 models. By recreating the operations of all hardware components at a low level, the emulator achieves a high degree of compatibility with original CPC software. These programs or games can be run unmodified at real-time or higher speeds, depending on the emulator host environment.

# Features

Caprice32 provides:
  * Complete emulation of CPC464, CPC664 and CPC6128
  * Mostly working support of Plus Range: CPC464+/CPC6128+/GX4000 (missing vectored & DMA interrupts, analog joysticks and 8 bit printer)
  * Joystick support - it can be fully used with joystick only, thanks to an integrated virtual keyboard.
  * Joystick emulation - joystick-only games can be played using the keyboard
  * English, French or Spanish keyboards
  * DSK, [IPF](http://softpres.org/glossary:ipf) and CT-RAW files for disks - VOC and CDT files for tapes - CPR files for cartridge
  * Snapshots (SNA files)
  * Direct load of ZIP files
  * Developers' tools with debugger, memory editor, disassembler...
  * Custom disk formats
  * Printer support
  * Experimental support of Multiface 2 (you should prefer using memory tool)
  * Net4CPC (W5100S Ethernet) emulation — TCP and UDP sockets backed by host POSIX sockets

You see something missing ? Do not hesitate to open an issue to suggest it.

# Installation

## macOS

See the [INSTALL.md](INSTALL.md)

## Linux

### Debug behavior and release behavior when locating configuration file

If you compile Caprice32 yourself with plain make, behavior is
debug-oriented.  By default at run-time it will look for `cap32.cfg`
in the *current directory of the process* that launches it, not in the
executable location as stated in the documentation.  To get the
documented behavior, use `APP_PATH` like in the examples below.

### From Git

```
git clone https://github.com/ColinPitrat/caprice32.git
cd caprice32
make APP_PATH="$PWD"
./cap32
```

### From releases

Download a release from https://github.com/ColinPitrat/caprice32/releases.
Decompress it and then from a terminal in the resulting directory:

```
make APP_PATH="$PWD"
./cap32
```

### Snap

A SNAP (maintained by a third party) is available at https://snapcraft.io/caprice32.

## Windows

Download a release from https://github.com/ColinPitrat/caprice32/releases.
Decompress it and double click on cap32.exe

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

# Net4CPC Ethernet emulation

This fork adds emulation of the [Net4CPC](https://github.com/salvogendut/net4cpc) expansion board, which connects a WIZnet W5100S Ethernet controller to the CPC expansion bus.

## Enabling it

Open the emulator menu (F1) → **Options** → **General** tab → tick **Enable Net4CPC ethernet emulation**. The setting is saved in `cap32.cfg` as `net4cpc=1`.

## How it works

The W5100S is accessed through four Z80 I/O ports using the chip's indirect parallel-bus protocol:

| Port   | Name    | Role |
|--------|---------|------|
| 0xFD20 | MR      | Mode Register — returns 0x03 (chip present, auto-increment on) |
| 0xFD21 | IDM_ARH | High byte of 16-bit indirect address |
| 0xFD22 | IDM_ARL | Low byte of 16-bit indirect address |
| 0xFD23 | IDM_DR  | Data register — address auto-increments after each access |

The full 64 KB W5100S register space is emulated in software:

| Range          | Contents |
|----------------|----------|
| 0x0000–0x002F  | Common registers (MR, GAR, SUBR, SHAR, SIPR, …) |
| 0x0400–0x07FF  | Socket 0–3 registers (256 bytes each) |
| 0x4000–0x5FFF  | TX ring buffers (2 KB per socket) |
| 0x6000–0x7FFF  | RX ring buffers (2 KB per socket) |

Socket commands (OPEN, CONNECT, SEND, RECV, CLOSE, …) are intercepted and mapped to host POSIX sockets — TCP via `SOCK_STREAM`, UDP via `SOCK_DGRAM`. All sockets are non-blocking; connection completion and incoming data are detected lazily when the CPC firmware reads the socket status or RX size registers.

For UDP receive, the emulator prepends the standard W5100S 8-byte header to each datagram in the RX ring buffer (4 bytes source IP + 2 bytes source port + 2 bytes payload length), matching the behaviour expected by W5100S firmware such as the Net4CPC DNS library.

## Source IP binding (SIPR)

When the CPC firmware writes its desired IP address into the W5100S SIPR registers (0x000F–0x0012) and issues an OPEN command, the emulator calls `bind()` on the host socket to that address. This allows the emulated CPC to appear on the local network under its own IP, provided that IP is assigned to a host interface. If the bind fails the socket falls back to the host's default IP silently.

## Limitations

- No hardware interrupts (IR pin) — firmware must poll Sn_SR / Sn_RX_RSR.
- TCP listen / server-side accept is not implemented.
- ICMP (ping) is not emulated.
- The emulation is only available on Linux/macOS builds (uses POSIX socket APIs).

# Why another GitHub repository ?

There are many repositories for caprice32 on GitHub:

  * https://github.com/burzumishi/caprice32
  * https://github.com/rofl0r/caprice32
  * https://github.com/MrZammler/caprice32
  * https://github.com/burzumishi/caprice32wx

So why create another one ? All these repositories are highly inactive. The ones that touched the code added dependencies (wxWidget, GTK) without really adding features.
