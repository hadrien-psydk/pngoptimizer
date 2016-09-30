#!/bin/bash
POVER=2.5

# Set current directory to the one containing this script
cd "$(dirname "$0")"
mkdir -p pngoptimizer-$POVER
cd pngoptimizer-$POVER || HandleFail
echo $PWD

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
echo "========== Packaging PngOptimizer =========="
make -C ../../projects/pngoptimizer/ CONFIG=release

POSRC=../../projects/pngoptimizer
PODEB=deb/pngoptimizer_$POVER-1_amd64
rm -rf $PODEB
mkdir -p $PODEB
mkdir -p $PODEB/DEBIAN
mkdir -p $PODEB/usr/bin
mkdir -p $PODEB/usr/share/icons/hicolor/128x128/apps
mkdir -p $PODEB/usr/share/applications

cp -v "$POSRC/linux-release/pngoptimizer" "$PODEB/usr/bin/" || HandleFail
cp -v "$POSRC/gtk/logo128.png"            "$PODEB/usr/share/icons/hicolor/128x128/apps/pngoptimizer.png" || HandleFail
cp -v "$POSRC/gtk/pngoptimizer.desktop"   "$PODEB/usr/share/applications/" || HandleFail

SIZE=$(du -sk $PODEB | cut -f 1)

cat > "$PODEB/DEBIAN/control" <<EOL
Package: pngoptimizer
Version: $POVER-1
Section: base
Priority: optional
Architecture: amd64
Maintainer: Hadrien Nilsson <pngoptimizer@psydk.org>
Installed-Size: $SIZE
Homepage: http://pngoptimizer.org
Description: PngOptimizer
 Optimize PNGs and convert other lossless formats to PNG
EOL

dpkg-deb --build $PODEB
# Move the .deb in the parent directory
mv -v $PODEB.deb ./

###############################################################################
# PngOptimizerCL
echo ""
echo "========== Packaging PngOptimizerCL =========="
make -C ../../projects/pngoptimizercl/ CONFIG=release

mkdir -p linux/pngoptimizercl
cp -v "../../projects/pngoptimizercl/linux-release/pngoptimizercl"  "linux/pngoptimizercl/" || HandleFail
cp -v "../../projects/pngoptimizercl/Readme.txt"                    "linux/pngoptimizercl/" || HandleFail
cp -v "../../projects/pngoptimizercl/License.txt"                   "linux/pngoptimizercl/" || HandleFail
cp -v "../../projects/pngoptimizercl/Changelog.txt"                 "linux/pngoptimizercl/" || HandleFail

###############################################################################
# Create PngOptimizerCL archive.
# We first need to fix file permissions that are not handled by
# the NTFS partition.
# To do that we copy the files to /tmp and change the permissions there
# before creating the tgz archive
TGZFILE="pngoptimizercl-$POVER-linux-x64.tgz"

tput setaf 3
pushd .
rm -rf /tmp/pngotmp
mkdir /tmp/pngotmp || HandleFail
cp -avr linux/pngoptimizercl /tmp/pngotmp || HandleFail
cd /tmp/pngotmp/
chmod 0755 pngoptimizercl
chmod 0644 pngoptimizercl/*
chmod 0755 pngoptimizercl/pngoptimizercl
echo "Creating $TGZFILE..."
tar -pczvf "$TGZFILE" "pngoptimizercl" || HandleFail
popd
tput sgr0

cp -v "/tmp/pngotmp/$TGZFILE" "." || HandleFail

