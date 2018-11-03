# This the Makefile fragment to be included by SDK, Projects and UnitTests Makefiles
#
# Makefile inputs:
# PROJECT_NAME    same as directory name
# PROJECT_TYPE    lib or app
# PROJECT_FILES   all files or wildcard like *.cpp
# SDK_DEPS        needed library names in sdk/
# EXT_DEPS        external libraries, like gtk got GTK+
# INCDIRS         additional include directories
# DESTDIR         optional directory when installing/uninstalling

# Command line options:
# (default)       builds all with debug configuration
# CONFIG=release  builds all with release configuration
# CONFIG=coverage builds all with coverage configuration
# clean           cleans current project and its dependencies
# rebuild         cleans then builds
# run             runs the application
# cov             creates coverage files. make CONFIG=coverage should have been called
# install         install the application
# uninstall       uninstall the application

INCDIRS += ../../sdk
CONFIG ?= debug
OUTDIR := linux-$(CONFIG)

GCC := gcc
GPP := g++
LD  := ld
OBJCOPY := objcopy

CFLAGS := -Wall
CFLAGS += -fdata-sections -ffunction-sections -fvisibility=hidden
CFLAGS += -fPIC
LDFLAGS := --gc-sections,--no-export-dynamic
EXTLIBS := -pthread -lrt

# In Debug, store debug symbols (-g)
# In Release, optimize and strip symbols with the linker
ifeq ($(CONFIG),debug)
	CFLAGS += -g
else ifeq ($(CONFIG),release)
	CFLAGS += -O2 -Werror
	LDFLAGS := $(LDFLAGS),-s
else ifeq ($(CONFIG),coverage)
	CFLAGS += -g -fprofile-arcs -ftest-coverage
	EXTLIBS += -lgcov
else
$(error Bad CONFIG value)
endif

CPPFLAGS := $(CFLAGS) -Wextra -Wshadow
# Uncomment for tests. gtk and gtest make it difficult
# CPPFLAGS +=-Wzero-as-null-pointer-constant
CPPFLAGS += -Wlogical-op -Woverloaded-virtual
CPPFLAGS += -std=c++0x $(addprefix -I,$(INCDIRS))

# Most includes for GTK+ are in /usr/include but some of them are
# very specific. pkg-config gives us all the needed paths.
ifeq ($(EXT_DEPS),gtk)
	CPPFLAGS += $(shell pkg-config --cflags gtk+-3.0)
	EXTLIBS += $(shell pkg-config --libs gtk+-3.0)
endif

ifeq ($(PROJECT_TYPE), lib)
	OUTPATH := $(OUTDIR)/lib$(PROJECT_NAME).a
else
	OUTPATH := $(OUTDIR)/$(PROJECT_NAME)
	SDK_DIRS := $(addprefix ../../sdk/,$(SDK_DEPS))
	SDK_LIBDIRS := $(addsuffix /linux-$(CONFIG)/,$(SDK_DIRS))
	SDK_LIBNAMES := $(addprefix lib,$(SDK_DEPS))
	SDK_LIBNAMES := $(addsuffix .a,$(SDK_LIBNAMES))
	SDK_LIBPATHS := $(join $(SDK_LIBDIRS),$(SDK_LIBNAMES))
endif

PCHPATH := $(OUTDIR)/stdafx.h.gch

PROJECT_FILES := $(foreach filename,$(PROJECT_FILES),$(wildcard $(filename)))
OBJS := $(subst .cpp,.o, $(PROJECT_FILES))
OBJS := $(subst .c,.o, $(OBJS))
OBJS := $(subst .png,.png.o, $(OBJS))
OBJS := $(subst .glade,.glade.o, $(OBJS))
SUBDIRS := $(dir $(OBJS))
SUBDIRS := $(sort $(SUBDIRS))
SUBDIRS := $(subst ./,, $(SUBDIRS))
SUBDIRS := $(addprefix $(OUTDIR)/,$(SUBDIRS))
OBJS := $(addprefix $(OUTDIR)/,$(OBJS))

#$(info $(OBJS))
#$(info $(SUBDIRS))
#$(info $(SDK_LIBPATHS))
#$(info $(CFLAGS))

.PHONY: clean build_deps

all: build

$(info ----------------------- $(PROJECT_NAME) $(CONFIG) -----------------------)

sdkdir = $(dir $(patsubst %/,%,$(dir $1)))

$(SDK_LIBPATHS): %:
	@$(MAKE) --no-print-directory -C $(call sdkdir,$@)

build: $(OUTPATH)

clean:
ifneq ($(PROJECT_TYPE), lib)
	@for dir in $(SDK_DIRS); do $(MAKE) --no-print-directory -C $$dir clean; done
endif
	$(info cleaning...)
	@rm -rf $(OUTDIR)

$(OUTDIR):
	@mkdir -p $(OUTDIR)

$(SUBDIRS): $(OUTDIR)
	@mkdir -p $@

$(PCHPATH): $(SDK_LIBPATHS) stdafx.h
	$(info $(PROJECT_NAME) - stdafx.h (precompiled header))
	@$(GPP) $(CPPFLAGS) -c stdafx.h -o $(PCHPATH)

$(OBJS): $(PCHPATH)

$(OUTPATH): $(OUTDIR) $(SUBDIRS) $(PCHPATH) $(OBJS)
ifeq ($(PROJECT_TYPE), lib)
	$(info creating library $(OUTPATH))
	@ar rcs $(OUTPATH) $(OBJS)
else
	$(info linking application $(OUTPATH))
	@$(GPP) $(OBJS) $(SDK_LIBPATHS) -Wl,$(LDFLAGS) $(EXTLIBS) -o $(OUTPATH)
endif

-include $(subst .o,.d, $(OBJS))

$(OUTDIR)/%.o: %.cpp $(PCHPATH)
	$(info $(PROJECT_NAME) - $<)
	@$(GPP) $(CPPFLAGS) -c -MMD -MP $< -o $@

$(OUTDIR)/%.o: %.c
	$(info $(PROJECT_NAME) - $<)
	@$(GCC) $(CFLAGS) -c -MMD -MP $< -o $@

$(OUTDIR)/%.png.o: %.png
	$(info $(PROJECT_NAME) - $< (resource))
	@$(LD) -r -o $@ -z noexecstack --format=binary $<
	@$(OBJCOPY) --rename-section .data=.rodata,alloc,load,readonly,data,contents $@

$(OUTDIR)/%.glade.o: %.glade
	$(info $(PROJECT_NAME) - $< (resource))
	@$(LD) -r -o $@ -z noexecstack --format=binary $<
	@$(OBJCOPY) --rename-section .data=.rodata,alloc,load,readonly,data,contents $@

rebuild: clean all

run: all
	$(OUTPATH) $(ARGS)

install: all
# We only create the directory tree if we are installing for a package
ifneq ($(DESTDIR),)
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/icons/hicolor/16x16/apps
	mkdir -p $(DESTDIR)/usr/share/icons/hicolor/48x48/apps
	mkdir -p $(DESTDIR)/usr/share/icons/hicolor/128x128/apps
	mkdir -p $(DESTDIR)/usr/share/applications
	mkdir -p $(DESTDIR)/usr/share/doc
endif

	cp $(OUTPATH) $(DESTDIR)/usr/bin/

ifeq ($(EXT_DEPS),gtk)
	cp gtk/logo16.png              $(DESTDIR)/usr/share/icons/hicolor/16x16/apps/$(PROJECT_NAME).png
	cp gtk/logo48.png              $(DESTDIR)/usr/share/icons/hicolor/48x48/apps/$(PROJECT_NAME).png
	cp gtk/logo128.png             $(DESTDIR)/usr/share/icons/hicolor/128x128/apps/$(PROJECT_NAME).png
	cp gtk/$(PROJECT_NAME).desktop $(DESTDIR)/usr/share/applications/

# We only refresh the system icon cache when actually installing in the system
ifeq ($(DESTDIR),)
	@echo updating icon cache...
	gtk-update-icon-cache          $(DESTDIR)/usr/share/icons/hicolor
endif
endif

# Documentation
	mkdir -p $(DESTDIR)/usr/share/doc/$(PROJECT_NAME)
	cp Readme.txt   $(DESTDIR)/usr/share/doc/$(PROJECT_NAME)
	cp Changelog.txt   $(DESTDIR)/usr/share/doc/$(PROJECT_NAME)
	cp License.txt   $(DESTDIR)/usr/share/doc/$(PROJECT_NAME)

	@echo installed

.PHONY: uninstall
uninstall:
	rm $(DESTDIR)/usr/bin/$(PROJECT_NAME)
ifeq ($(EXT_DEPS),gtk)
	rm $(DESTDIR)/usr/share/icons/hicolor/16x16/apps/$(PROJECT_NAME).png
	rm $(DESTDIR)/usr/share/icons/hicolor/48x48/apps/$(PROJECT_NAME).png
	rm $(DESTDIR)/usr/share/icons/hicolor/128x128/apps/$(PROJECT_NAME).png
	rm $(DESTDIR)/usr/share/applications/$(PROJECT_NAME).desktop
	rm -rf $(DESTDIR)/usr/share/doc/$(PROJECT_NAME)
endif
	@echo uninstalled

# -----------------------------------------------------------------------------
# Unit test code coverage
# sudo apt install lcov
OTHER_GCDA_DIRS := $(addprefix --directory ,$(SDK_DIRS))

cov: linux-coverage/html/index.html
	xdg-open linux-coverage/html/index.html

linux-coverage/html/index.html: linux-coverage/coverage.info
	genhtml linux-coverage/coverage.info --output-dir linux-coverage/html

# .gcda files should be present. They are created when executing the application.
# Just check one .gcda presence to know if running should be done
linux-coverage/coverage.info: linux-coverage/stdafx.gcda
	lcov --capture --directory linux-coverage $(OTHER_GCDA_DIRS) --output-file linux-coverage/coverage.info

linux-coverage/stdafx.gcda: linux-coverage/$(PROJECT_NAME)
	linux-coverage/$(PROJECT_NAME)
