# eCos makefile

# This is a generated file - do not edit

export HOST := CYGWIN
export PREFIX := /ecos-c/cygwin/home/andrew/gps/redboot/redboot_rom/untitled_install
export COMMAND_PREFIX := arm-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export AR := $(COMMAND_PREFIX)ar

.PHONY: default build clean tests headers

build: headers $(PREFIX)/include/pkgconf/ecos.mak
	$(MAKE) -r -C hal/arm/arch/current arm.inc
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/gps4020/current $@
	$(MAKE) -r -C devs/flash/atmel/at49xxxx/current $@
	$(MAKE) -r -C devs/flash/arm/gps4020/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C io/flash/current $@
	$(MAKE) -r -C hal/common/current $(PREFIX)/lib/extras.o
	$(MAKE) -r -C hal/arm/arch/current $(PREFIX)/lib/vectors.o
	$(MAKE) -r -C hal/arm/arch/current $(PREFIX)/lib/target.ld
	$(MAKE) -r -C redboot/current $(PREFIX)/bin/redboot.elf
	$(MAKE) -r -C hal/arm/gps4020/current $(PREFIX)/bin/redboot.bin
	@echo $@ finished

clean:
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/gps4020/current $@
	$(MAKE) -r -C devs/flash/atmel/at49xxxx/current $@
	$(MAKE) -r -C devs/flash/arm/gps4020/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C io/flash/current $@
	@echo $@ finished

tests: build
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/gps4020/current $@
	$(MAKE) -r -C devs/flash/atmel/at49xxxx/current $@
	$(MAKE) -r -C devs/flash/arm/gps4020/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C io/flash/current $@
	@echo $@ finished

headers:
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/gps4020/current $@
	$(MAKE) -r -C devs/flash/atmel/at49xxxx/current $@
	$(MAKE) -r -C devs/flash/arm/gps4020/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C io/flash/current $@
	@echo $@ finished

$(PREFIX)/include/pkgconf/ecos.mak: makefile
	@echo 'ECOS_GLOBAL_CFLAGS = -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority' > $@
	@echo 'ECOS_GLOBAL_LDFLAGS = -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib' >> $@
	@echo 'ECOS_COMMAND_PREFIX = $(COMMAND_PREFIX)' >> $@

