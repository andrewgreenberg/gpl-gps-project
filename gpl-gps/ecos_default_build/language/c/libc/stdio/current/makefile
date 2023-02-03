# eCos makefile

# This is a generated file - do not edit

export PREFIX := /ecos-c/cygwin/home/andrew/gps/gpl-gps/ecos_default_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := CYGWIN
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /opt/ecos-cvs/packages
PACKAGE := language/c/libc/stdio/current
OBJECT_PREFIX := language_c_libc_stdio
CFLAGS := -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc
LDFLAGS := -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/sprintf1 tests/sprintf2 tests/sscanf tests/stdiooutput 

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/common/fflush.cxx src/common/feof.cxx src/common/freopen.cxx src/common/setvbuf.cxx src/common/snprintf.cxx src/common/sprintf.cxx src/common/sscanf.cxx src/common/stderr.cxx src/common/stdin.cxx src/common/stdiofiles.cxx src/common/fseek.cxx src/common/stdioinlines.cxx src/common/stdiosupp.cxx src/common/stdout.cxx src/common/stream.cxx src/common/streambuf.cxx src/common/ungetc.cxx src/common/vsnprintf.cxx src/common/vsscanf.cxx src/common/fileops.cxx src/input/fgetc.cxx src/input/fgets.cxx src/input/fread.cxx src/input/fscanf.cxx src/input/gets.cxx src/input/scanf.cxx src/input/vfscanf.cxx src/output/fnprintf.cxx src/output/fprintf.cxx src/output/fputc.cxx src/output/fputs.cxx src/output/fwrite.cxx src/output/printf.cxx src/output/vfnprintf.cxx src/common/fopen.cxx src/common/fclose.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/libc/stdio/io.hxx $(PREFIX)/include/cyg/libc/stdio/io.inl $(PREFIX)/include/cyg/libc/stdio/stdio.h $(PREFIX)/include/cyg/libc/stdio/stdio.inl $(PREFIX)/include/cyg/libc/stdio/stdiofiles.hxx $(PREFIX)/include/cyg/libc/stdio/stdiofiles.inl $(PREFIX)/include/cyg/libc/stdio/stdiosupp.hxx $(PREFIX)/include/cyg/libc/stdio/stream.hxx $(PREFIX)/include/cyg/libc/stdio/stream.inl $(PREFIX)/include/cyg/libc/stdio/streambuf.hxx $(PREFIX)/include/cyg/libc/stdio/streambuf.inl

$(PREFIX)/include/cyg/libc/stdio/io.hxx: $(REPOSITORY)/$(PACKAGE)/include/io.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/io.inl: $(REPOSITORY)/$(PACKAGE)/include/io.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stdio.h: $(REPOSITORY)/$(PACKAGE)/include/stdio.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stdio.inl: $(REPOSITORY)/$(PACKAGE)/include/stdio.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stdiofiles.hxx: $(REPOSITORY)/$(PACKAGE)/include/stdiofiles.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stdiofiles.inl: $(REPOSITORY)/$(PACKAGE)/include/stdiofiles.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stdiosupp.hxx: $(REPOSITORY)/$(PACKAGE)/include/stdiosupp.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stream.hxx: $(REPOSITORY)/$(PACKAGE)/include/stream.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/stream.inl: $(REPOSITORY)/$(PACKAGE)/include/stream.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/streambuf.hxx: $(REPOSITORY)/$(PACKAGE)/include/streambuf.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/stdio/streambuf.inl: $(REPOSITORY)/$(PACKAGE)/include/streambuf.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

