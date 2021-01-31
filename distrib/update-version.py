#!/usr/bin/python3

# This script updates the version inside many files of the repository

import os, sys, re, time
import subprocess
from pathlib import Path

def cat(path):
	with open(Path(path)) as f:
		data = f.read()
		f.close()
	return data

def replace_between(path, start, stop, new):
	data = cat(path)

	start = start.replace(")", "\\)");
	start = start.replace("(", "\\(");

	patt = "(" + start + ")"
	repl = "\\g<1>" + new
	any = '.*'
	if stop:
		stop = stop.replace(")", "\\)");
		stop = stop.replace("(", "\\(");
		if stop == ' ':
			any = '[^ ]*'
		stop = stop.replace(" ", "\\b");
		patt += any
		patt += "(" + stop + ")"
		repl += "\\g<2>"
	else:
		patt += any

	(data2, rc) = re.subn(patt, repl, data, 1, re.MULTILINE)
	if rc != 1:
		raise Exception("No replacement done in " + path)
		return False

	'''
	# Regex testing
	print(patt)
	with open('/tmp/xx.txt', 'wb') as output:
		output.write(data2.encode('utf-8'))
	output.close()

	subprocess.call('diff ' + path + ' /tmp/xx.txt', shell=True)
	return True

	'''
	with open(path, 'w', newline="\n") as output:
		output.write(data2)
		output.close()
	return True

# -----------------------------------------------------------------------------
# Set current directory to the directory of this script
dname = os.path.dirname(__file__)
os.chdir(dname)

# This is the version we will use
version = cat("../VERSION").strip()

replace_between("make-distrib.sh", "POVER=", "", version)
replace_between("make-distrib.bat", "POVER=", "", version)

os.chdir("../projects")

replace_between("pngoptimizer/Readme.txt", r"Version\s*:\s*", "", version)
replace_between("pngoptimizer/Changelog.txt",
	"^-----------------\n2[0-9][0-9][0-9].*(", ")", version)
replace_between("pngoptimizer/msgtable.h", r'#define PNGO_VERSION\s*"', '"', version)
replace_between("pngoptimizer/gtk/pngoptimizer.desktop", "Version=", "", version)

replace_between("pngoptimizercl/Readme.txt", r"Version\s*:\s*", "", version)
replace_between("pngoptimizercl/Changelog.txt",
	"^-----------------\n2[0-9][0-9][0-9].*(", ")", version)
replace_between("pngoptimizercl/main.cpp", r'#define PNGO_VERSION\s*"', '"', version)

# Update also the year
ts = os.path.getmtime("../VERSION")
year = time.strftime('%Y', time.gmtime(ts))

replace_between("pngoptimizer/Readme.txt", '2002/', ' ', year)
replace_between("pngoptimizer/msgtable.h", '2002/', ' ', year)

replace_between("pngoptimizercl/Readme.txt", '2002/', ' ', year)
replace_between("pngoptimizercl/main.cpp", '2002/', ' ', year)

print("success")
