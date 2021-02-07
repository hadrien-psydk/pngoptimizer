# PngOptimizer ![PngOptimizer logo](projects/pngoptimizer/gtk/logo48.png)

A graphics and command line tools to optimize PNGs and convert other lossless formats (GIF, BMP, TGA) into optimized PNGs.

## How to build

### Linux:

#### Prerequisites
Install build tools. On Ubuntu, open a terminal then execute:
 `sudo apt install build-essential`

For PngOptimizer, install GTK+ 3 development library:
 `sudo apt install libgtk-3-dev`

#### Quick full build and install
This option is only available on Linux. Open a terminal then execute:
  `make`
  `sudo make install`

These commands build and install both PngOptimizer and PngOptimizerCL in release mode.

#### Individual builds
Individual builds build in debug mode by default.
 1. Open a terminal
 2. Change directory to projects/pngoptimizer/
 3. Execute command: `make` or `make CONFIG=release`

    These commands build in debug and release configuration respectively.
    The build results are located in the linux-debug/ or linux-release/ directories.

On Linux, `make` accept other targets: clean, run, install, uninstall, cov. Open sdk/chulib.mk for a description at the top of the file.

For PngOptimizerCL, do the same except that you should change directory to projects/pngoptimizercl instead.

### Windows
 1. Use Microsoft Visual Studio 2019 and open projects/pngoptimizer/PngOptimizer.sln
 2. Build the solution, in either Debug or Release mode, and for Win32 (x86) or x64.

The final build result, PngOptimizer.exe, is located into the Win32-Debug, Win32-Release,
x64-Debug or x64-Release sub-directory of the PngOptimizer directory.

For PngOptimizerCL, do the same except that you should change directory to projects/pngoptimizercl instead.

### Build directories
The SDK libraries have subdirectories where build files are stored:
 * Win32-Debug for the Windows 32-bit debug version
 * Win32-Release for the Windows 32-bit release version
 * x64-Debug for the Windows 64-bit debug version
 * x64-Release for the Windows 64-bit release version
 * linux-debug for Linux 64-bit debug version
 * linux-release for Linux 64-bit release version

## Packaging
To build all and create the packages to be downloaded, call from a terminal:
 * On Windows: `distrib/make-distrib.bat`
 * On Linux: `distrib/make-distrib.sh`

Packages will be created in the distrib/download directory.

On Linux, to build all and install all you can also use the top level Makefile. This is useful when creating a binary Debian package.

## License information
 * The PngOptimizer application in the "projects" directory
  is distributed under the terms of the GNU General Public Licence.
 * The PngOptimizerCL application in the "projects" directory
  is distributed under the terms of the GNU General Public Licence.
 * The poeng library in the "sdk" directory
  is distributed under the terms of the GNU Lesser General Public Licence.
 * The chuwin32 library in the "sdk" directory
  is distributed under the terms of the Zlib licence.
 * The chustd library in the "sdk" directory
  is distributed under the terms of the Zlib licence.
 * The chustd uses the Zlib library, distributed under the terms of the
  ZLib licence.
 * The unit_tests directory uses the Google Test framework, distributed under
  the terms of the new BSD license.
