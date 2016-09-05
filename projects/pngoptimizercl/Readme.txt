-----------------------------------------------------------------------------
  PngOptimizerCL - Copyright (C) 2002/2016 Hadrien Nilsson - psydk.org
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
Version   : 2.5-beta
Home page : http://pngoptimizer.org
Contact   : pngoptimizer@psydk.org

This is the command line version of PngOptimizer.
Please report to the PngOptimizer Readme.txt file for more information.

Run pngoptimizercl with no argument to display the usage.

== About the KeepBackgroundColor option ==

By default, PngOptimizerCL will remove any background color information found in a PNG (bkGD chunk).
If the option is provided, the background color information will be kept.

KeepBackgroundColor can also receive an argument:

- Use KeepBackgroundColor:R (or 0) to remove any background color information (same as no KeepBackgroundColor option)
- Use KeepBackgroundColor:K (or 1) to keep background color information if present (default argument)
- Use KeepBackgroundColor:F (or 2) to force a background color. The color is specificed with
  the BkColor option and a color in hexadecimal RGB format. Example:

pngoptimizercl -file:"myfile.png" -KeepBackgroundColor:F -BkColor:FF425A

== About the KeepTextualData option ==

By default, PngOptimizerCL will remove any textual data information found in a PNG.
If the option is provided, the textual information found as tEXt chunks will be kept.
Note that other types of text chunks are currently not supported and will be removed (like iTXt chunks).

KeepTextualData can also receive an argument:

- Use KeepTextualData:R (or 0) to remove tXTt chunks (as if no option provided)
- Use KeepTextualData:K (or 1) to keep tXTt chunks
- Use KeepTextualData:F (or 2) to remove all tXTt chunks and force one, using a key/data pair provided
  with the ForcedTextKeyword and ForcedTextData options. Example:

pngoptimizercl -file:"myfile.png" -KeepTextualData:F -ForcedTextKeyword:Author -ForcedTextData:"Hadrien Nilsson"
