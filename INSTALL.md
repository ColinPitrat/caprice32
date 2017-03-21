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

# IPF support

Caprice32 supports IPF file format (http://www.softpres.org/).
To enable it, you need to first download the proper package from http://softpres.org/download (under "User Distribution").
You need version 4.2 at least.

#### On Linux

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

Then build Caprice with WITH_IPF:

`make WITH_IPF=true`

You can then run an IPF file as you would do for a DSK file:

`./cap32.exe disk.ipf`

Note that zip file are not yet supported.

#### On Windows

IPF is not yet supported by Caprice32 on Windows. But this should come soon so stay tuned.

