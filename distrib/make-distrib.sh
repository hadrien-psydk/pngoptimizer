#!/bin/bash
POVER=2.5

# Set current directory to the one containing this script
cd "$(dirname "$0")"

function HandleFail
{
	tput setaf 1
	echo "="
	echo "=   FAILURE"
	echo "="
	tput sgr0
	exit 1
}

function HandleSuccess
{
	tput setaf 2
	echo "="
	echo "=   SUCCESS"
	echo "="
	tput sgr0
	exit 0	
}

###############################################################################
# PngOptimizer
echo -e "\e[36m========== Packaging PngOptimizer ==========\e[0m"
make -C ../projects/pngoptimizer/ CONFIG=release

PODEBDIR=deb/pngoptimizer_$POVER-1_amd64
rm -rf $PODEBDIR
mkdir -p $PODEBDIR
mkdir -p $PODEBDIR/DEBIAN

make -C ../projects/pngoptimizer/ CONFIG=release DESTDIR=../../distrib/$PODEBDIR install

SIZE=$(du -sk $PODEBDIR | cut -f 1)

cat > "$PODEBDIR/DEBIAN/control" <<EOL
Package: pngoptimizer
Version: $POVER-1
Architecture: amd64
Maintainer: Hadrien Nilsson <pngoptimizer@psydk.org>
Section: graphics
Priority: optional
Installed-Size: $SIZE
Homepage: http://pngoptimizer.org
Description: PngOptimizer
 Optimize PNGs and convert other lossless formats to PNG
EOL

tput setaf 3
echo "Creating $PODEBDIR.deb..."
dpkg-deb --build $PODEBDIR
# Move the .deb in the parent directory
mv -v $PODEBDIR.deb ./
tput sgr0

###############################################################################
# PngOptimizerCL
echo ""
echo -e "\e[36m========== Packaging PngOptimizerCL ==========\e[0m"
make -C ../projects/pngoptimizercl/ CONFIG=release

POCLDIR=tgz/pngoptimizercl
rm -rf $POCLDIR
mkdir -p $POCLDIR

cp -v "../projects/pngoptimizercl/linux-release/pngoptimizercl"  "$POCLDIR" || HandleFail
cp -v "../projects/pngoptimizercl/Readme.txt"                    "$POCLDIR" || HandleFail
cp -v "../projects/pngoptimizercl/License.txt"                   "$POCLDIR" || HandleFail
cp -v "../projects/pngoptimizercl/Changelog.txt"                 "$POCLDIR" || HandleFail

TGZFILE="pngoptimizercl-$POVER-linux-x64.tgz"

tput setaf 3
cd tgz
echo "Creating $TGZFILE..."
tar -pczvf "$TGZFILE" "pngoptimizercl" || HandleFail
cd ..
tput sgr0


