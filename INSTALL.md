# Prerequisites

You will need the following to successfully compile an executable:

  * MinGW (only for Windows) - http://sourceforge.net/projects/mingw/
  * SDL - http://www.libsdl.org/index.php
  * FreeType - https://www.freetype.org/
  * zLib - http://www.gzip.org/zlib/
  * libPNG - http://libpng.org/pub/png/libpng.html

# Compiling

### Linux/macOS target:

To build with default options, use :

`make`

The following options are available:

 * `DEBUG=TRUE`
 * `WITHOUT_GL=TRUE`
 * `ARCH=(win32|win64|linux)`

For example, for a linux debug build, use:

`make DEBUG=TRUE`

Alternatively, the debug target also work:

`make debug`

### How to compile on macOS

This was tested for macOS Sequoia 15.1.

You will need SDL installed. Install brew if you don't have it. Then:

`brew install SDL`

Use make to build:

`make`

After a while you will get the executable.

The usual way to organize disk/tape/cartridge images and snapshots is to create directories `disk`, `tape`, `cart` and `snap`:

`mkdir disk tape cart snap`

Start the emulator with:

`./cap32`

or

`./cap32 <file>`

where `file` can be a `.dsk`, `.voc`, `.cdt`, `.cpr`, `.sna` or a `.zip` containing one of these files.

See "Using the emulator" below for information on how to use Caprice.

### Troubleshooting

If you encounter an error like:
```
/usr/include/SDL/SDL_opengl.h:116: error: "GL_GLEXT_VERSION" redefined [-Werror]
```

This is caused by a conflict between a `gl_ext.h` file provided by SDL and the
one provided by your OpenGL implementation. This can be fixed by not using the
one provided by SDL:

```
make CFLAGS=-DNO_SDL_GLEXT
```

### Example build on a Debian 10 System

Download a debian install image e.g. from https://www.debian.org/CD/netinst/
and install debian 10 on a virtual or real machine. Create your user account
during installation.

Add your user to the sudoer group.

```
su -
usermod -aG sudo <user>
# now you need to logout and login again!
```

Install git.

`sudo apt-get install git`

Set your proxy variables (if necessary) and clone the caprice repository.

```
export HTTP_PROXY="http://proxy.company.com:9999"
export HTTPS_PROXY=$HTTP_PROXY

git clone https://github.com/ColinPitrat/caprice32.git
cd caprice32
```

Install g++, make, sdl2 and freetype.

`sudo apt-get install g++ make libsdl2-dev libfreetype6-dev`

Finally build the linux binary.

`make ARCH=linux`

To speed up the build you can use more than one processes if you have more cores, e.g.

`make ARCH=linux -j 4` 

Test the binary

`./cap32`

### Debian/Ubuntu package:

To build a debian package on Debian/Ubuntu distributions, install the dependencies as mentioned above and the debian packaging helper packages:

`sudo apt-get install dpkg-dev pbuilder quilt devscripts fakeroot debhelper`

Then, to create a debian package:

 * fill in the required version in the debian/changelog file
 * compile with `make VERSION=<my_version_string>`
 * go to `release/cap32_linux/caprice32-<version>/debian`
 * execute `debuild -us -uc --lintian-opts --profile debian` or `pdebuild` if you want to run in a chrooted env.

### Windows target:

To build with default options for 32 bits architecture (i686), use:

`make ARCH=win32`

and for 64 bits (x86_64) use:

`make ARCH=win64`

You may need to edit the makefile to update `MINGW_PATH`, `CXX`, `IPATHS` and `LIBS` for windows platform with the correct values for your installation of mingw.


### Example Build for mingw64 on Windows10

Download MSys2.0 from https://www.msys2.org.
We assume that you are using 64bit architecture,
so you need to download msys2-x86_64-xxxxxxxx.exe (msys2-x86_64-20190524.exe). 

Install it under default folder "c:\msys64". 
Do not automatically start the msys2 shell after the installation is complete.

Open folder "c:\msys64" and start "mingw64.exe".

If you are working behind a firewall, you might need to define the 
http proxy for the packet manager. For this enter the proper export commands.
```
export HTTP_PROXY="username:password@proxy.server.address:port"
export HTTPS_PROXY=$HTTP_PROXY
export http_proxy=$HTTP_PROXY
export https_proxy=$HTTP_PROXY
```

Update the package manager database.

`pacman -Syu`

When you find the message "Warnung: terminate MSYS2 without returning to shell
and check for updates again" then close the window with the close symbol and repeat the previous step.

`pacman -Syu`

Install git.

`pacman -S git`

Clone the caprice32 repository.

`git clone https://github.com/ColinPitrat/caprice32.git`

Install the 64 bit gcc compiler.

`pacman -S mingw-w64-x86_64-gcc`

Install make.

`pacman -S make`

Adjust the win64 part in the makefile.

```
ifeq ($(ARCH),win64)
TRIPLE = x86_64-w64-mingw32
PLATFORM=windows
MINGW_PATH = /mingw64
```

Adjust the windows part in the makefile.

```
ifeq ($(PLATFORM),windows)
TARGET = cap32.exe
TEST_TARGET = test_runner.exe
IPATHS = -Isrc/ -Isrc/gui/includes -I$(MINGW_PATH)/include -I$(MINGW_PATH)/include/SDL2 -I$(MINGW_PATH)/include/freetype2
LIBS = $(MINGW_PATH)/lib/libSDL2.dll.a $(MINGW_PATH)/lib/libfreetype.dll.a $(MINGW_PATH)/lib/libz.dll.a $(MINGW_PATH)/lib/libpng16.dll.a $(MINGW_PATH)/lib/libpng.dll.a
COMMON_CFLAGS = -DWINDOWS
CXX = $(MINGW_PATH)/bin/g++
```

Install SDL 2

`pacman -S mingw-w64-i686-SDL2`

Install freetype.

`pacman -S mingw-w64-x86_64-freetype`

Install zip.

`pacman -S zip`

Finally build the caprice binary.

`make ARCH=win64`

Test the fresh Caprice32 Build.

`./cap32`

If you want to build the 32 bit version you need to adjust the MINGW_PATH in the makefile and to install the proper packages for i686 (32 bit) instead of x86_64 e.g.

`pacman -S mingw-w64-i686-gcc`


#### On Linux/macOS

Unpack the package and copy the library and headers in a place where they will be found by the compilation.
Typically:

```
cd tmp
tar -xvzf ipflib42_linux-x86_64.tar.gz
cd x86_64-linux-gnu-capsimage/
sudo cp libcapsimage.so.4.2 /usr/lib
sudo ln -s /usr/lib/libcapsimage.so.4.2 /usr/lib/libcapsimage.so.4
sudo ln -s /usr/lib/libcapsimage.so.4.2 /usr/lib/libcapsimage.so
sudo cp -r include/caps /usr/include
sudo chmod a-x /usr/include/caps/* /usr/lib/libcapsimage.so*
sudo chmod a+r /usr/include/caps /usr/include/caps/* /usr/lib/libcapsimage.so*
```

#### On Windows

# Using the emulator

F1 provides the emulator's main menu. The `About` section contains a list of shortcuts.

To get a list of supported command line flags:
`cap32 --help` 

These flags can be useful to automate things.

Once in the emulator, you can type F1 and then "Load/Save" to:
 - Load an image of a disk, tape, cartridge
 - Load an emulator state from a snapshot
 - Insert a new (empty) disk
 - Save a disk
 - Save the emulator's state in a snapshot

By default, Caprice uses CPC 6128 which is on disk by default. To see what's on a disk loaded in drive A, type `cat`.

To start a program, type `run"program`.

You can find more details on the web. CPCWiki is full of nice resources, in particular the user manual which is a good start:
https://www.cpcwiki.eu/index.php/User_Manual

