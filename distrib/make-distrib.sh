#!/bin/bash
POVER=2.5

# Set current directory to the one containing this script
cd "$(dirname "$0")"

mkdir -p pngoptimizer-$POVER
cd pngoptimizer-$POVER || HandleFail
echo $PWD
mkdir -p linux/pngoptimizercl

make -C ../../projects/pngoptimizercl/ CONFIG=release

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

cp -v "../../projects/pngoptimizer/linux-release/pngoptimizer"  "linux/pngoptimizer/" || HandleFail
cp -v "../../projects/pngoptimizer/Readme.txt"                  "linux/pngoptimizer/" || HandleFail
cp -v "../../projects/pngoptimizer/License.txt"                 "linux/pngoptimizer/" || HandleFail
cp -v "../../projects/pngoptimizer/Changelog.txt"               "linux/pngoptimizer/" || HandleFail

###############################################################################
# PngOptimizerCL

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

