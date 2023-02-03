/*---------------------------------------------------------------------------\
| Project:	gp4020_toggle_port @ http://gps.psas.pdx.edu/
|
| File:		gp4020_toggle_port.S
|
| Description:	Toggles GPIO[7] on the GP4020. Meant to be loaded via the
|		GP4020's bootloader in ROM.
|
| Building:	See Makefile and read_me.txt
|
|    Date    | Notes
|------------|------------------------------------------------------------
| 08/18/2003 | Moved from calling gcc to calling gas, added linker call,
|            | cleaned up big time.
| 08/04/2003 | Further simplified to the most basic program
| 07/23/2003 | Created. Thanks to John Zaitseff
|            | <J.Zaitseff@unsw.edu.au> for creating the ARM assembly
|            | template!
\---------------------------------------------------------------------------*/

	.equ	GPIO_BASE_ADDRESS,	 	0xE0005000
	.equ	GPIO_DIRECTION_REG_OFFSET,	0x00000000
	.equ	GPIO_READ_REG_OFFSET,		0x00000004
	.equ	GPIO_WRITE_REG_OFFSET,		0x00000008

/*--------------------------------------------------------------------------*/
@ Set up assembler

	.text

	.arm		@ AKA ".code32"
	.align 2 	@ Align code on 2^2 = 4 byte intervals

/*--------------------------------------------------------------------------*/
@ Setup the GPIO and the ARM registers

@ make GPIO[7] an output

	mov	r1, #(GPIO_BASE_ADDRESS & 0xFF000000)
	add	r1, r1, #(GPIO_BASE_ADDRESS & 0x0000FF00)
	add	r1, r1, #GPIO_DIRECTION_REG_OFFSET
	ldr	r0, [r1]
	and	r0, r0, #0b01111111	@ 0 = output, 1 = intput
	str	r0, [r1]		@ Store value (DON'T use strb)

@ Set up r1 to point to the GPIO write register

	mov	r1, #(GPIO_BASE_ADDRESS & 0xFF000000)
	add	r1, r1, #(GPIO_BASE_ADDRESS & 0x0000FF00)
	add	r1, r1, #GPIO_WRITE_REG_OFFSET
	ldr	r0, [r1]	@ Load in current state of the output latch

/*--------------------------------------------------------------------------*/
@ Loop forever, toggling GPIO[7] without munging the other output values

loop:
	orr	r0, r0, #0b10000000		@ GPIO[7] = 1
	str	r0, [r1]
	and	r0, r0, #0b01111111		@ GPIO[7] = 0
	str	r0, [r1]

	b	loop

/*--------------------------------------------------------------------------*/

	.end
