#########################################################################
#									#
#                Makefile for Assembly Language Program                 #
#									#
#########################################################################

# Author:   John Zaitseff <J.Zaitseff@unsw.edu.au>, Modified by
#           Andrew Greenberg (andrew@uad.com)
# Date:     18th August, 2003
# Version:  1.0

# Please note that every line that is indented (or, in the language of
# Makefiles, the command part of the rules) is done so with TAB
# characters, NOT with spaces.  This is important and a subtle trap for
# the unwary!


all:		gp4020_echo.ldr

gp4020_echo.ldr:	gp4020_echo.bin
gp4020_echo.bin:	gp4020_echo.elf
gp4020_echo.elf:	gp4020_echo.o
gp4020_echo.o:		gp4020_echo.s


clean:
	-rm -f gp4020_echo.ldr gp4020_echo.bin gp4020_echo.elf gp4020_echo.o

# The following variables and implicit rules are required for the GNU
# Assembler for ARM.  You probably do not need to modify anything here.

AS        = arm-elf-as
LD        = arm-elf-ld
OC        = arm-elf-objcopy

# ADG: we don't need debugging info: removed "ASFLAGS   = --gdwarf2"
ASFLAGS   =
LDFLAGS   =
OCFLAGS   = --output-target binary
LOADLIBES =
LDLIBS    =

.SUFFIXES:

# Assemble ARM assembly language source (.s) to an object file (.o) using as.
# eg: "arm-elf-as -marm7tdmi gp4020_toggle_port.s -o gp4020_toggle_port.o"

%.o: %.s
	$(AS) -marm7tdmi $(ASFLAGS) $< -o $@

# Link object file (.0) into an ARM executable (.elf), using ld.
# eg: "arm-elf-ld -o gp4020_toggle_port.o gp4020_toggle_port.elf"

%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

# "Copy" ELF file (.elf) into a binary file for execution using objcopy.
# eg: "arm-elf-objcopy --output-target binary gp4020_toggle_port.elf gp4020_toggle_port.bin"

%.bin: %.elf
	$(OC) $(OCFLAGS) $< $@

# Take the binary file and using a small C program called 'size-enc' which should be in the same
# directory as the Makefile, add three bytes of length to the top of the binary for the GP4020
# bootloader.

%.ldr: %.bin
	-./size-enc $< > $@
	-cat $< >> $@

# Miscellaneous rules

.PHONY:	all clean
.DEFAULT:
.SUFFIXES:
