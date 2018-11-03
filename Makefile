# Top level Makefile to build and install everything for end-user
# It builds both PngOptimizer and PngOptimizerCL

# To ease development, default configuration is debug for individual
# Makefiles, but for the end-user we select release as the default.
export CONFIG=release

.PHONY: all clean install

all:
	$(MAKE) -C projects/pngoptimizer
	$(MAKE) -C projects/pngoptimizercl

clean:
	$(MAKE) -C projects/pngoptimizer clean

install:
	$(MAKE) -C projects/pngoptimizer install
