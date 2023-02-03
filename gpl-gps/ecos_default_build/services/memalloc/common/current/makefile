# eCos makefile

# This is a generated file - do not edit

export PREFIX := /ecos-c/cygwin/home/andrew/gps/gpl-gps/ecos_default_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := CYGWIN
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /opt/ecos-cvs/packages
PACKAGE := services/memalloc/common/current
OBJECT_PREFIX := services_memalloc_common
CFLAGS := -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc
LDFLAGS := -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/dlmalloc1 tests/dlmalloc2 tests/heaptest tests/kmemfix1 tests/kmemvar1 tests/malloc1 tests/malloc2 tests/malloc3 tests/malloc4 tests/memfix1 tests/memfix2 tests/memvar1 tests/memvar2 tests/realloc tests/sepmeta1 tests/sepmeta2

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/dlmalloc.cxx src/memfixed.cxx src/memvar.cxx src/sepmeta.cxx src/debug.c src/kapi.cxx src/malloc.cxx heaps.o.d
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

heaps.o.d: heaps.cxx
	$(CC) $(ACTUAL_CXXFLAGS) $(INCLUDE_PATH) -Wp,-MD,heaps.tmp -c -o $(OBJECT_PREFIX)_$(notdir $(@:.o.d=.o)) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" heaps.tmp > $@
	@rm heaps.tmp
	

heapgeninc.tcl: $(wildcard $(REPOSITORY)/$(PACKAGE)/src/heapgen.cpp)
	$(CC) $(ACTUAL_CXXFLAGS) $(INCLUDE_PATH) -Wp,-MD,heapgen.tmp -E $< -o $@
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" heapgen.tmp > $(notdir $@).deps
	@rm heapgen.tmp
	

heaps.cxx: $(wildcard heapgeninc.tcl $(REPOSITORY)/$(PACKAGE)/src/heapgen.tcl)
	XPWD=`pwd` ; cd $(REPOSITORY)/$(PACKAGE)/src ; sh heapgen.tcl "$(PREFIX)" "$$XPWD"
	@cp heaps.hxx "$(PREFIX)"/include/pkgconf/heaps.hxx
	@chmod u+w "$(PREFIX)"/include/pkgconf/heaps.hxx
	

headers: mlt_headers $(PREFIX)/include/cyg/memalloc/common.hxx $(PREFIX)/include/cyg/memalloc/dlmalloc.hxx $(PREFIX)/include/cyg/memalloc/dlmallocimpl.hxx $(PREFIX)/include/cyg/memalloc/kapi.h $(PREFIX)/include/cyg/memalloc/kapidata.h $(PREFIX)/include/cyg/memalloc/memfixed.hxx $(PREFIX)/include/cyg/memalloc/memjoin.hxx $(PREFIX)/include/cyg/memalloc/memjoin.inl $(PREFIX)/include/cyg/memalloc/mempolt2.hxx $(PREFIX)/include/cyg/memalloc/mempolt2.inl $(PREFIX)/include/cyg/memalloc/mempoolt.hxx $(PREFIX)/include/cyg/memalloc/mempoolt.inl $(PREFIX)/include/cyg/memalloc/memvar.hxx $(PREFIX)/include/cyg/memalloc/mfiximpl.hxx $(PREFIX)/include/cyg/memalloc/mfiximpl.inl $(PREFIX)/include/cyg/memalloc/mvarimpl.hxx $(PREFIX)/include/cyg/memalloc/mvarimpl.inl $(PREFIX)/include/cyg/memalloc/sepmeta.hxx $(PREFIX)/include/cyg/memalloc/sepmetaimpl.hxx $(PREFIX)/include/cyg/memalloc/sepmetaimpl.inl

$(PREFIX)/include/cyg/memalloc/common.hxx: $(REPOSITORY)/$(PACKAGE)/include/common.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/dlmalloc.hxx: $(REPOSITORY)/$(PACKAGE)/include/dlmalloc.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/dlmallocimpl.hxx: $(REPOSITORY)/$(PACKAGE)/include/dlmallocimpl.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/kapi.h: $(REPOSITORY)/$(PACKAGE)/include/kapi.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/kapidata.h: $(REPOSITORY)/$(PACKAGE)/include/kapidata.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/memfixed.hxx: $(REPOSITORY)/$(PACKAGE)/include/memfixed.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/memjoin.hxx: $(REPOSITORY)/$(PACKAGE)/include/memjoin.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/memjoin.inl: $(REPOSITORY)/$(PACKAGE)/include/memjoin.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mempolt2.hxx: $(REPOSITORY)/$(PACKAGE)/include/mempolt2.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mempolt2.inl: $(REPOSITORY)/$(PACKAGE)/include/mempolt2.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mempoolt.hxx: $(REPOSITORY)/$(PACKAGE)/include/mempoolt.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mempoolt.inl: $(REPOSITORY)/$(PACKAGE)/include/mempoolt.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/memvar.hxx: $(REPOSITORY)/$(PACKAGE)/include/memvar.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mfiximpl.hxx: $(REPOSITORY)/$(PACKAGE)/include/mfiximpl.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mfiximpl.inl: $(REPOSITORY)/$(PACKAGE)/include/mfiximpl.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mvarimpl.hxx: $(REPOSITORY)/$(PACKAGE)/include/mvarimpl.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/mvarimpl.inl: $(REPOSITORY)/$(PACKAGE)/include/mvarimpl.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/sepmeta.hxx: $(REPOSITORY)/$(PACKAGE)/include/sepmeta.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/sepmetaimpl.hxx: $(REPOSITORY)/$(PACKAGE)/include/sepmetaimpl.hxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/memalloc/sepmetaimpl.inl: $(REPOSITORY)/$(PACKAGE)/include/sepmetaimpl.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

