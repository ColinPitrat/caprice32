This has been integrated from https://www.kryoflux.com/?page=download

Steps taken:
 - Unzip both windows and linux file in 2 different directories
 - Copy one of the 2 as capsimg and keep only the .cpp/.h files from it
 - Diff windows & linux/macos version and in the capsimg/ copy, replace diffs with appropriate #ifdef/ifndef WINDOWS
 - Remove Compatibility directory (which contains only dirent.h)
 - Remove CAPSImg/dllmain.cpp
 - Create the `ignore_warnings.h` file
 - Add `#include "ignore_warnings.h"` in files that require it to compile. Extend the list of ignored warnings if necessary.
