# eCos makefile

# This is a generated file - do not edit

export PREFIX := /ecos-c/cygwin/home/andrew/gps/redboot/redboot_ram/redboot_ram_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := CYGWIN
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /opt/ecos-cvs/packages
PACKAGE := net/ns/dns/current
OBJECT_PREFIX := net_ns_dns
CFLAGS := -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority -D_KERNEL -D__ECOS
LDFLAGS := -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers

headers: mlt_headers $(PREFIX)/include/cyg/ns/dns/dns.h $(PREFIX)/include/cyg/ns/dns/dns_impl.inl $(PREFIX)/include/cyg/ns/dns/dns_priv.h

$(PREFIX)/include/cyg/ns/dns/dns.h: $(REPOSITORY)/$(PACKAGE)/include/dns.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/ns/dns/dns_impl.inl: $(REPOSITORY)/$(PACKAGE)/include/dns_impl.inl
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/ns/dns/dns_priv.h: $(REPOSITORY)/$(PACKAGE)/include/dns_priv.h
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

