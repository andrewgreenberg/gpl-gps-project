-----------------------------------------------------------------------------
Files for the gp4020_echo program
Andrew Greenberg
October 5th, 2003
-----------------------------------------------------------------------------

This program prints "Hello,world!" and then echos received characters on the
UART and to the GPIO of the GP4020 ARM7TDMI/GPS receiver.

Files included in this tar:

Makefile	- Type "make" to build the .o/.elf/.bin/.ldr files.

gp4020_echo.s	- Assembly Source
gp4020_echo.o  	- Object file
gp4020_echo.elf	- ELF file.
gp4020_echo.bin	- Binary file.
gp4020_echo.ldr	- Binary file with length prepended; for GP4020 boot loader.

size-enc	- Program that prints length of a file in three bytes of hex
		  (Used in the make process to make the .ldr files)
size-enc.c	- Source for size-enc


