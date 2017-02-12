-----------------------------------------------------------------------------
  PngOptimizer - Copyright (C) 2002/2017 Hadrien Nilsson - psydk.org
-----------------------------------------------------------------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
-----------------------------------------------------------------------------

Platform  : Windows/Linux - x86/x64
Licence   : GNU GPL 2
Version   : 2.5.1
Home page : http://psydk.org/pngoptimizer
Contact   : pngoptimizer@psydk.org

Contributions : 
- PngOptimizer icon by Guillaume Schaeffer
- Beta testing, bug reports : many friends & contributors, thanks for all!

Thank you for downloading PngOptimizer. If you like PngOptimizer, please
consider making a donation through the PayPal button on the home page.

The source code can be downloaded from the PngOptimizer home page.

-----------------------------------------------------------------------------
== Purpose ==

The goals of PngOptimizer are :
- to clean PNG files of useless information, or wrong information
- to make PNG files smaller
- to convert other lossless image formats to PNG
- to create a PNG file from a screenshot and make it easily available
- to perform all those tasks in the simpliest and the most productive way

== Usage ==

To clean, convert and optimize image files :
 Drop files from the file explorer into PngOptimizer main window. The files 
 are converted and optimized immediately. If the compression of a PNG cannot
 be improved, the file will be just cleaned.

To paste a screenshot :
 Press PrintScreen or Alt+PrintScreen, then press Ctrl+V or Shift+Insert in 
 PngOptimizer main window. You can then double-click on the created link to 
 view the result, and drag the link to another application.

For advanced PNG options, perform a right click to display a context menu.

== PNG options ==

* Backup old PNG files *
 If you are not confident enough with PngOptimizer, this option lets you 
 rename your old PNG files with an underscore ( _ ) at the beginning of their 
 name. Otherwise the previous file is replaced by the optimized one.

* Keep interlacing *
 Use this option if you do not want PngOptimizer to remove interlacing in 
 your PNG file. Interlacing is nice but it increases file size.

* Avoid grey with simple transparency *

 This is intended for Web developers dealing with Internet Explorer 6.

 Let's say you have an image with two colors, the first color is a totally
 opaque black, and the second one is totally transparent, then this image is
 a good candidate for a GrayScale PNG with 1 bit per pixel and a chunk that
 says that white means total transparency.

 Internet Explorer 6 is unable to display correctly those kinds of PNG. That
 option lets you avoid this kind of optimization.

* Ignore animated GIFs *

 If a GIF is animated, PngOptimizer will convert it to APNG (Animated PNG).
 If you enble this option, PngOptimzer will ignore the file.

* Keep file date *

 When enabled, reuse the file date (Last modification date) of the original file
 for the new file.

* Background color (bkGD chunk) *
 
 This is intended for Web developers dealing with Internet Explorer 6.
 
 In Internet Explorer 6, PNG images with an alpha channel are blended by
 default with a grey background color, or if provided, by a background 
 color specified in the PNG.

 You can choose to remove, keep, or force a background color. Note that
 forcing a background color is limited to PNGs which are in 32 bits RGBA.

* Textual data (tEXt chunk) *

 This option lets you remove all, keep all or force one textual data.
 
* Physical pixel dimensions (pHYs chunk) *

 This option lets you remove all, keep all or force information related to the
 physical size of pixels. Some systems use this information to offer an optimal
 display according to the pixel definition of the screen or printer.

== Screenshot options ==

* Paste screenshot

 If you prefer using the menu than a shortcut key, this item is for you.

* Show screenshots directory

 Opens a file explorer window showing the screenshots directory.

* Screenshots options

 Opens a dialog box to set screenshots options :

  - the screenshots location : the temporary directory in your profile 
    directory or a specific location.

  - an option to feed the optimization engine with the screenshots you make.
    When set, you will get smaller files, but you will have to wait longer.

  - an option to ask for a screenshot file name when you paste it into 
    PngOptimizer. By default, the screenshot file name contains the date and
    time it was taken. If PngOptimizer finds some kind of index, it will propose
    a new file name with an incremented index. For example: file 1.png, file 2.png...


== Where are stored PngOptimizer settings ? ==

By default, the settings are stored into a file in your user profile directory.
For example :
C:\Documents and Settings\Joe\Application Data\PngOptimizer\PngOptimizer.ini

If a PngOptimizer.ini file is present in the PngOptimizer.exe directory, that
.ini file will be used instead and no specific directory will be created in
your user profile directory. This way you can carry PngOptimizer along with
your settings on a portable storage, like an USB stick.

Before PngOptimizer 1.6, the settings were stored in the registry. If you
were using older versions of PngOptimizer, you can clean your registry by
deleting the key HKEY_CURRENT_USER\Software\Darken Utilities.


== Some notes about the optimization engine ==

When optimzing, PngOptimizer tries to decrease the number of bits per pixel.
It means that a 32 bits image may become a 24 bits image, a 24 bits image
may become a 8 bits indexed image, a 8 bits indexed image may become a 
4 bits indexed image...

What PngOptimizer specifically avoids is increasing the number of bits per
pixels when transparency is involved. For example, an indexed image will not
be converted to a 32 bits image even if it can lead to a better compression.

The reason is that when you have a bunch a transparent indexed PNG files, you
usually want to keep them indexed so they can still be displayed correctly in
Internet Explorer 6.

