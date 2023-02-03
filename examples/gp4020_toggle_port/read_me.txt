-----------------------------------------------------------------------------
Files for the gp4020_toggle_port program
Andrew Greenberg
August 18th, 2003
-----------------------------------------------------------------------------

Makefile		- Type "make" to build the .o/.elf/.bin files.

gp4020_toggle_port.bin	- Binary file. NEEDS LENGTH FOR BOOT LOADER.
gp4020_toggle_port.elf  - ELF file.
gp4020_toggle_port.o   	- Object file
gp4020_toggle_port.s	- Assembly Source

gp4020_toggle_port_w_length.bin - This file has the length of the binary
file prepended to it in the first three bytes (0x00 0x00 0x3c) so that
it can be dumped right out the serial port and run on the GP4020's serial
boot loader.


