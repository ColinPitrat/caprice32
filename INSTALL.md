# Prerequisites

You will need the following to successfully compile an executable:

  * MinGW (only for Windows) - http://sourceforge.net/projects/mingw/
  * SDL - http://www.libsdl.org/index.php
  * FreeType - https://www.freetype.org/
  * zLib - http://www.gzip.org/zlib/
  * libPNG - http://libpng.org/pub/png/libpng.html

# Compiling

#### Linux target:

To build with default options, use :

`make`

The following options are available:

 * `DEBUG=TRUE`
 * `WITHOUT_GL=TRUE`
 * `ARCH=(win32|win64|linux)`

For example, for a debug build, use:

`make DEBUG=TRUE`

Alternatively, the debug target also work:

`make debug`

#### Debian/Ubuntu package:

To build a debian package on Debian/Ubuntu distributions install the dependencies as mentioned above and the dpkg-dev package:

`sudo apt-get install dpkg-dev`

Then build the package with the following command:

`dpkg-buildpackage -rfakeroot -uc -us`

After installation of the package the users should copy the /etc/cap32.cfg file to their homedirectory with the following command:

`cp /etc/cap32.cfg .cap32.cfg`

#### Windows target:

To build with default options for 32 bits architecture (i686), use:

`make ARCH=win32`

and for 64 bits (x86_64) use:

`make ARCH=win64`

You may need to edit the makefile to update `MINGW_PATH`, `CXX`, `IPATHS` and `LIBS` for windows platform with the correct values for your installation of mingw.

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
