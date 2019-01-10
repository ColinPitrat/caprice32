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
 * `WITH_IPF=TRUE`
 * `ARCH=(win32|win64|linux)`

For example, for a linux debug build, use:

`make DEBUG=TRUE`

Alternatively, the debug target also work:

`make debug`

#### Debian/Ubuntu package:

To build a debian package on Debian/Ubuntu distributions, install the dependencies as mentioned above and the debian packaging helper packages:

`sudo apt-get install dpkg-dev pbuilder quilt devscripts fakeroot debhelper`

Then, to create a debian package:

 * fill in the required version in the debian/changelog file
 * compile with `make VERSION=<my_version_string>`
 * go to `release/cap32_linux/caprice32-<version>/debian`
 * execute `debuild -us -uc --lintian-opts --profile debian` or `pdebuild` if you want to run in a chrooted env.

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

`./cap32 disk.ipf`

#### On Windows

The windows builds available in releases include IPF support.

