#!/usr/bin/python3

# This script updates the version inside many files of the repository

import os, sys, re

def cat(path):
	with open(path) as f:
		data = f.read()
	f.close()
	return data

def replace_between(path, start, stop, new):
	data = cat(path)

	start = start.replace(")", "\\)");
	start = start.replace("(", "\\(");

	patt = "(" + start + ")" + ".*"
	repl = "\\g<1>" + new
	if stop:
		stop = stop.replace(")", "\\)");
		stop = stop.replace("(", "\\(");
		patt += "(" + stop + ")"
		repl += "\\g<2>"

	(data2, rc) = re.subn(patt, repl, data, 1, re.MULTILINE)
	if rc != 1:
		raise Exception("No replacement done in " + path)
		return False

	'''
	# Regex testing
	output = sys.stdout
	output.write(data2)
	return True
	'''

	with open(path, 'wb') as output:
		output.write(data2.encode('utf-8'))
	output.close()
	return True

# -----------------------------------------------------------------------------
# Set current directory to the directory of this script
dname = os.path.dirname(__file__)
os.chdir(dname)

# This is the version we will use
version = cat("../VERSION")

replace_between("make-distrib.sh", "POVER=", "", version)
replace_between("make-distrib.bat", "POVER=", "", version)

os.chdir("../projects")

replace_between("pngoptimizer/Readme.txt", r"Version\s*:\s*", "", version)
replace_between("pngoptimizer/Changelog.txt",
	"^-----------------\n2[0-9][0-9][0-9].*(", ")",	version)
replace_between("pngoptimizer/msgtable.h", r'#define PNGO_VERSION\s*"', '"', version)
replace_between("pngoptimizer/gtk/pngoptimizer.desktop", "Version=", "", version)

replace_between("pngoptimizercl/Readme.txt", r"Version\s*:\s*", "", version)
replace_between("pngoptimizercl/Changelog.txt",
	"^-----------------\n2[0-9][0-9][0-9].*(", ")",	version)
replace_between("pngoptimizercl/main.cpp", r"#define PNGO_VERSION\s*", "", version)

print("success")
