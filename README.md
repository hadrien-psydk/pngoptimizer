# PngOptimizer ![PngOptimizer logo](projects/pngoptimizer/gtk/logo48.png)

A graphics and command line tools to optimize PNGs and convert other lossless formats (GIF, BMP, TGA) into optimized PNGs.

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

--------------------------------------------------------------------------
How to build PngOptimizer?

For Windows:

 1. Use Microsoft Visual Studio 2015 and open projects/pngoptimizer/PngOptimizer.sln
 2. Build the solution, in either Debug or Release mode, and for Win32 (x86) or x64.

The final build result, PngOptimizer.exe, is located into the Win32-Debug, Win32-Release,
x64-Debug or x64-Release sub-directory of the PngOptimizer directory.

For Linux:

 1. Open a terminal
 2. Ensure you have build tools. To get the build tools on Ubuntu,
    open a terminal and execute this command: `sudo apt install build-essential`
 3. Ensure you have GTK+ development package: `sudo apt install libgtk-3-dev`
 4. Change directory to projects/pngoptimizer/
 5. Execute command: `make` or `make CONFIG=release`

    These commands build in debug and release configuration respectively.
    The build results are located in the linux-debug/ or linux-release/ directories.

On Linux, `make` accept other targets: clean, run, install, uninstall, cov. Open sdk/chulib.mk for a description at the top of the file.

The SDK libraries have subdirectories where build files are stored:

 * Win32-Debug for the Windows 32-bit debug version
 * Win32-Release for the Windows 32-bit release version
 * x64-Debug for the Windows 64-bit debug version
 * x64-Release for the Windows 64-bit release version
 * linux-debug for Linux 64-bit debug version
 * linux-release for Linux 64-bit release version
 * linux-coverage for Linux 64-bit debug+coverage version (used when calling make cov)


--------------------------------------------------------------------------
How to build PngOptimizerCL?

For Windows: same as PngOptimizer except you need to open projects/pngoptimizercl/PngOptimizerCL.sln

For Linux: same as PngOptimizer except you need to change directory to projects/pngoptimizercl, and the GTK+ development package is not needed.

--------------------------------------------------------------------------
To build all and create the packages to be downloaded, call from a terminal:

 * On Windows: `distrib/make-distrib.bat`
 * On Linux: `distrib/make-distrib.sh`

Packages will be created in the distrib/download directory.

On Linux, to build all and install all you can also use the top level Makefile. This is usefull when creating a Debian package.


