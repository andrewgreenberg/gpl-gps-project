# eCos makefile

# This is a generated file - do not edit

export PREFIX := /ecos-c/cygwin/home/andrew/gps/gpl-gps/ecos_default_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := CYGWIN
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /opt/ecos-cvs/packages
PACKAGE := language/c/libc/string/current
OBJECT_PREFIX := language_c_libc_string
CFLAGS := -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc
LDFLAGS := -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/memchr tests/memcmp1 tests/memcmp2 tests/memcpy1 tests/memcpy2 tests/memmove1 tests/memmove2 tests/memset tests/strcat1 tests/strcat2 tests/strchr tests/strcmp1 tests/strcmp2 tests/strcoll1 tests/strcoll2 tests/strcpy1 tests/strcpy2 tests/strcspn tests/strcspn tests/strlen tests/strncat1 tests/strncat2 tests/strncpy1 tests/strncpy2 tests/strpbrk tests/strrchr tests/strspn tests/strstr tests/strtok tests/strxfrm1 tests/strxfrm2

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/memchr.cxx src/memcmp.cxx src/memmove.cxx src/strcat.cxx src/strchr.cxx src/strcmp.cxx src/strcoll.cxx src/strcpy.cxx src/strcspn.cxx src/strlen.cxx src/strncat.cxx src/strncmp.cxx src/strncpy.cxx src/strpbrk.cxx src/strrchr.cxx src/strspn.cxx src/strstr.cxx src/strsuppt.cxx src/strtok.cxx src/strxfrm.cxx src/bsdstring.cxx src/strdup.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/libc/string/bsdstring.h $(PREFIX)/include/cyg/libc/string/string.h $(PREFIX)/include/cyg/libc/string/string.inl $(PREFIX)/include/cyg/libc/string/stringsupp.hxx

$(PREFIX)/include/cyg/libc/string/bsdstring.h: $(REPOSITORY)/$(PACKAGE)/include/bsdstring.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/string/string.h: $(REPOSITORY)/$(PACKAGE)/include/string.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/string/string.inl: $(REPOSITORY)/$(PACKAGE)/include/string.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/string/stringsupp.hxx: $(REPOSITORY)/$(PACKAGE)/include/stringsupp.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

