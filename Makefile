### ---
## Filename: Makefile
## Author: Casey Gregoire <caseyg@lalosoft.com>
## Created: Feb 22 2019
## Version: 1.0.0
##
## Compatibility: GNUMakefile
##
######################################################################
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 3 of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
##
######################################################################

CC?=gcc

## Directories
RELEASE_DIR?=bin
IDIR:=inc
ODIR:=obj
SDIR:=src
DDIR:=dep
ALL_DIRS=$(RELEASE_DIR) $(IDIR) $(SDIR) $(DDIR) $(ODIR)
TEMP_DIRS=$(RELEASE_DIR) $(DDIR) $(ODIR)

# vpath directives
# Headers
vpath %.h $(SDIR)
vpath %   $(IDIR)

# Source
vpath %.c $(SDIR)

# Objects
vpath %.o $(ODIR)

# Depends
vpath %.d $(DDIR)

## Semantics
LIBS+= glib-2.0

LDFLAGS+=$(shell pkg-config --libs $(LIBS))
ALL_LDFLAGS=$(LDFLAGS)

CFLAGS+=-Wall
CFLAGS+=$(shell pkg-config --cflags $(LIBS))
ALL_CFLAGS=-I$(IDIR) $(CFLAGS)

ALLFLAGS=$(ALL_CFLAGS) $(ALL_LDFLAGS)

COMPILE.c=$(CC) $(ALL_CFLAGS) -c -o $@ $<
LINK.o=$(CC) $(ALL_CFLAGS) $^ $(ALL_LDFLAGS) -o $(RELEASE_DIR)/$@

SRCS=$(wildcard $(SDIR)/*.c)
HDRS=$(wildcard $(SDIR)/*.h)
OBJS=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRCS))
DEPS=$(patsubst $(SDIR)/%.c,$(DDIR)/%.d,$(SRCS))
SRCFILES=$(SRCS) $(HDRS)


DEVICE_MODE_LIBS=
DEVICE_MODE_OBJS=$(ODIR)/tw6869-device-mode.o

APPS:= tw6869-device-mode

all: $(APPS)

## Creates src.d per source file
# 1st sed cmd moves :
# 2nd sed cmd adds $(ODIR) before the object
$(DDIR)/%.d: %.c
	@mkdir -p $(@D)

ifdef V
	@echo Generating $(@F)
endif
	@set -e; rm -f $@; \
	$(CC) $(ALL_CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ | \
	sed 's,$*.o,$(ODIR)/$*.o,g' > $@; \
	rm -f $@.$$$$

# create object files
define create-obj
	@mkdir -p $(@D)

	$(if $V,@echo "--DEBUG $1 ------------------------",
		@echo compiling file: $<)
	$(if $V,$(COMPILE.c),@$(COMPILE.c))
	$(if $V,@echo "--//DEBUG $1 ----------------------")
endef

$(ODIR)/%.o: %.c
	$(call create-obj,$(@F))

# Only call from within the context of linking
define dbg-link
	@mkdir -p $(RELEASE_DIR)

	$(if $V,@echo "--DEBUG $1 ------------------------",
		@echo linking files: $^)
	$(if $V,$(LINK.o) $2,@$(LINK.o) $2)
	$(if $V,@echo "--//DEBUG $1 ----------------------")
	@echo Executable in $(RELEASE_DIR)/$1
	@echo
endef

# Add App Targets here
	
tw6869-device-mode: $(DEVICE_MODE_OBJS)
	$(call dbg-link,"tw6869-device-mode")
	
	
.PHONY: clean tags etags
clean:
ifdef V
	@echo Cleaning files: $(wildcard $(ODIR)/*) \
			      $(wildcard $(DDIR)/*) \
			      $(wildcard $(RELEASE_DIR)/*)
endif
	@rm -rfv $(ODIR) $(DDIR) $(RELEASE_DIR)

	@exit 0

tags: etags
etags:
	etags $(SRCFILES) > TAGS

# Source New Makefiles.. generation occurs when required
-include $(DEPS)

######################################################################
### Makefile ends here
