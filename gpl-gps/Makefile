# Makefile for gpl-gps
# Copyright (C) 2005  Andrew Greenberg
# Distributed under the GNU GENERAL PUBLIC LICENSE (GPL) Version 2 (June 1991).
# See the "COPYING" file distributed with this software for more information.

ECOS_DIR = ecos_default_install

include $(ECOS_DIR)/include/pkgconf/ecos.mak

RM = rm -f
CC = $(ECOS_COMMAND_PREFIX)gcc
LD = $(CC)

# ECOS_GLOBAL_CFLAGS are defined in
# $(ECOS_DIR)/include/pkgconf/ecos.mak
# Currently they include -Wall -Wstrict-prototypes -g -O2
CXXFLAGS  = -I$(ECOS_DIR)/include $(ECOS_GLOBAL_CFLAGS)

# Removed -Wmissing-prototypes
# It's not clear function/data-sections are buying us anything.
# The other -f* flags are C++ specific, ecos may care, but probably not needed.
CFLAGS = -mcpu=arm7tdmi -save-temps -g3 -O2 -I$(ECOS_DIR)/include \
-ffunction-sections -fdata-sections -fno-exceptions \
-Wall -Wunused -Wpointer-arith -Wstrict-prototypes -Winline -Wundef

LDFLAGS   = -nostartfiles -L$(ECOS_DIR)/lib -Ttarget.ld -Wl,-Map=gpl-gps.map

.PHONY: all clean tags

all: gpl-gps

clean:
	-$(RM) gpl-gps *.o *.i *.s *.d *.map

tags:
	ctags *.[ch]

SRC = $(wildcard *.c)

# Rules
# Handle dependencies automatically in (GNU-make/gcc)-style
%.d: %.c
	$(CC) -isystem $(ECOS_DIR)/include -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Include auto-dependency files
-include $(SRC:.c=.d)

gpl-gps: $(SRC:.c=.o)
	$(LD) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS) $(SRC:.c=.o) -o $@

