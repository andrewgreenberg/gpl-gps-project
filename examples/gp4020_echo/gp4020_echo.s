/*---------------------------------------------------------------------------\
| Project:	gp4020_echo @ http://gps.psas.pdx.edu/
|
| File:		gp4020_echo.s
|
| Description:	Echos characters sent into UART0 out the GPIO and out
|		UART1. Meant to be loaded via the GP4020's bootloader
|               in ROM.
|
| Building:	See Makefile and read_me.txt
|
|    Date    | Notes
|------------|------------------------------------------------------------
| 10/04/2003 | Added echo to GPIOs to double check comm port working.
| 08/22/2003 | Created from gp4020_hello_world. Thanks to John Zaitseff
|            | <J.Zaitseff@unsw.edu.au> for creating the ARM assembly
|            | template!
\---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                          DEFINITIONS                                     */
/*--------------------------------------------------------------------------*/

@ GPIO defines

	.equ	GPIO_DIR,	0xE0005000
	.equ	GPIO_INPUT,	0xE0005004
	.equ	GPIO_OUTPUT,	0xE0005008

@ UART defines

	.equ	UART1_CR,	0xE0018000	@ Serial Control
	.equ	UART1_MR,	0xE0018004	@ Serial Mode
	.equ	UART1_BRR,	0xE0018008	@ Serial Baud Rate
	.equ	UART1_SR,	0xE001800C	@ Serial Status
	.equ	UART1_RR,	0xE0018010	@ Receive
	.equ	UART1_TR,	0xE0018010	@ Transmit
	.equ	UART1_MCR,	0xE0018014	@ Modem Control
	.equ	UART1_MSR,	0xE001801C	@ Modem Status

	.equ	UART2_CR,	0xE0019000	@ Serial Control
	.equ	UART2_MR,	0xE0019004	@ Serial Mode
	.equ	UART2_BRR,	0xE0019008	@ Serial Baud Rate
	.equ	UART2_SR,	0xE001900C	@ Serial Status
	.equ	UART2_RR,	0xE0019010	@ Receive
	.equ	UART2_TR,	0xE0019010	@ Transmit
	.equ	UART2_MCR,	0xE0019014	@ Modem Control
	.equ	UART2_MSR,	0xE001901C	@ Modem Status

/*--------------------------------------------------------------------------*/
/*                          PREAMBLE                                        */
/*--------------------------------------------------------------------------*/

	.text

	.arm		@ AKA ".code32"
	.align 2 	@ Align code on 2^2 = 4 byte intervals

/*--------------------------------------------------------------------------*/
/*                         Initialize GPIO                                  */
/*--------------------------------------------------------------------------*/

@ Set the GPIO pins to outputs

	mov	r1, #(GPIO_DIR & 0xFF000000)
	add	r1, r1, #(GPIO_DIR & 0x00FF0000)
	add	r1, r1, #(GPIO_DIR & 0x0000FF00)
	add	r1, r1, #(GPIO_DIR & 0x000000FF)
	mov	r0, #0b00010000		@ 0 = output, 1 = intput: LEAVE GPIO4 FLOATING FOR UART1 RX
	str	r0, [r1]		@ Store value (DON'T use strb)

@ Make them all low and set up r1 to point to the output register

	mov	r1, #(GPIO_OUTPUT & 0xFF000000)
	add	r1, r1, #(GPIO_OUTPUT & 0x00FF0000)
	add	r1, r1, #(GPIO_OUTPUT & 0x0000FF00)
	add	r1, r1, #(GPIO_OUTPUT & 0x000000FF)
	mov	r0, #0b10101010		@ Let the user know we're actually running
	str	r0, [r1]		@ Store value (DON'T use strb)

/*--------------------------------------------------------------------------*/
/*                         Initialize UART                                  */
/*--------------------------------------------------------------------------*/

	mov	r2, #(UART1_CR & 0xFF000000)
	add	r2, r2, #(UART1_CR & 0x00FF0000)
	add	r2, r2, #(UART1_CR & 0x0000FF00)
	add	r2, r2, #(UART1_CR & 0x000000FF)
	ldrb	r0,[r2]
	orr	r0, r0, #0b00000011
		 @         ||||||||- Receive Channel Control   : 1 = enable
		 @         |||||||-- Transmit Channel Control  : 1 = enable
		 @         ||||||--  Clock Source              : 0 = don't change
		 @         |||||---  Flow Control Type         : 0 = don't change
		 @         ||||----  Receive Interrupt Enable  : 0 = don't change
		 @         |||-----  Transmit Interrupt Enable : 0 = don't change
		 @         ||------  Error Interrupt Enable    : 0 = don't change
		 @         |-------  Modem Interrupt Enable    : 0 = don't change
	strb	r0, [r2]

@ Don't bother to set up the UART1_MR and UART1_BRR; will be set by the boot loader.

@ Set up UART2_CR, UART2_MR and UART2_BRR from UART1 settings

	mov	r2, #(UART1_MR & 0xFF000000)
	add	r2, r2, #(UART1_MR & 0x00FF0000)
	add	r2, r2, #(UART1_MR & 0x0000FF00)
	add	r2, r2, #(UART1_MR & 0x000000FF)
	ldrb	r0, [r2]
	mov	r2, #(UART2_MR & 0xFF000000)
	add	r2, r2, #(UART2_MR & 0x00FF0000)
	add	r2, r2, #(UART2_MR & 0x0000FF00)
	add	r2, r2, #(UART2_MR & 0x000000FF)
	strb	r0, [r2]

	mov	r2, #(UART1_BRR & 0xFF000000)
	add	r2, r2, #(UART1_BRR & 0x00FF0000)
	add	r2, r2, #(UART1_BRR & 0x0000FF00)
	add	r2, r2, #(UART1_BRR & 0x000000FF)
	ldrb	r0, [r2]
	mov	r2, #(UART2_BRR & 0xFF000000)
	add	r2, r2, #(UART2_BRR & 0x00FF0000)
	add	r2, r2, #(UART2_BRR & 0x0000FF00)
	add	r2, r2, #(UART2_BRR & 0x000000FF)
	strb	r0, [r2]

	mov	r2, #(UART1_CR & 0xFF000000)
	add	r2, r2, #(UART1_CR & 0x00FF0000)
	add	r2, r2, #(UART1_CR & 0x0000FF00)
	add	r2, r2, #(UART1_CR & 0x000000FF)
	ldrb	r0, [r2]
	mov	r2, #(UART2_CR & 0xFF000000)
	add	r2, r2, #(UART2_CR & 0x00FF0000)
	add	r2, r2, #(UART2_CR & 0x0000FF00)
	add	r2, r2, #(UART2_CR & 0x000000FF)
	strb	r0, [r2]

@ Load UART1 addresses in registers

	mov	r2, #(UART1_SR & 0xFF000000)
	add	r2, r2, #(UART1_SR & 0x00FF0000)
	add	r2, r2, #(UART1_SR & 0x0000FF00)
	add	r2, r2, #(UART1_SR & 0x000000FF)

	mov	r3, #(UART1_RR & 0xFF000000)
	add	r3, r3, #(UART1_RR & 0x00FF0000)
	add	r3, r3, #(UART1_RR & 0x0000FF00)
	add	r3, r3, #(UART1_RR & 0x000000FF)

	mov	r4, #(UART1_TR & 0xFF000000)
	add	r4, r4, #(UART1_TR & 0x00FF0000)
	add	r4, r4, #(UART1_TR & 0x0000FF00)
	add	r4, r4, #(UART1_TR & 0x000000FF)

@ Load UART2 addresses in registers

	mov	r5, #(UART2_SR & 0xFF000000)
	add	r5, r5, #(UART2_SR & 0x00FF0000)
	add	r5, r5, #(UART2_SR & 0x0000FF00)
	add	r5, r5, #(UART2_SR & 0x000000FF)

	mov	r6, #(UART2_RR & 0xFF000000)
	add	r6, r6, #(UART2_RR & 0x00FF0000)
	add	r6, r6, #(UART2_RR & 0x0000FF00)
	add	r6, r6, #(UART2_RR & 0x000000FF)

	mov	r7, #(UART2_TR & 0xFF000000)
	add	r7, r7, #(UART2_TR & 0x00FF0000)
	add	r7, r7, #(UART2_TR & 0x0000FF00)
	add	r7, r7, #(UART2_TR & 0x000000FF)


/*--------------------------------------------------------------------------*/
/*              Send out the universal project greeting                     */
/*--------------------------------------------------------------------------*/

@ on UART1

	adr	r8, phrase		@ Set up r5 to point to the TEXT
hw1_loop:
	ldrb	r0, [r8], #1		@ Load in a byte and r5++
	cmp	r0, #0			@ Is it zero?
	beq	hw1_done		@ Yes; we're done / No; send the byte out the UART
	strb	r0, [r4]		@ Store the TEXT byte in the transmit register
hw1_wait:
	ldrb	r0, [r2]		@ Poll the Status Register for a clear in the TX register status
	and	r0, r0, #0b0000010	@ Clear all bits but "Transmit Register Status"
	cmp	r0, #0			@ Do I need to do this?
	beq	hw1_wait
	b	hw1_loop		@ Keep looping until we're done with the TEXT

hw1_done:

@ On UART2

	adr	r8, phrase		@ Set up r5 to point to the TEXT
hw2_loop:
	ldrb	r0, [r8], #1		@ Load in a byte and r5++
	cmp	r0, #0			@ Is it zero?
	beq	hw2_done		@ Yes; we're done / No; send the byte out the UART
	strb	r0, [r7]		@ Store the TEXT byte in the transmit register
hw2_wait:
	ldrb	r0, [r5]		@ Poll the Status Register for a clear in the TX register status
	and	r0, r0, #0b0000010	@ Clear all bits but "Transmit Register Status"
	cmp	r0, #0			@ Do I need to do this?
	beq	hw2_wait
	b	hw2_loop		@ Keep looping until we're done with the TEXT

phrase:
	.ascii  "Hello, World!\n\r\0"

hw2_done:

/*--------------------------------------------------------------------------*/
/*                            Echo stuff                                    */
/*--------------------------------------------------------------------------*/

@ Loop forever, and take bytes received over the UART and echo them by
@ transmitting them back over the UART, and by displaying the byte on the GPIO pins.

echo_loop:

@ Check for received bytes on UART1

	ldrb	r0, [r2]		@ Load the status register (also resets error flags and rx/tx ready flags)
	and	r0, r0, #0b0000001	@ Clear all bits but "Receive Register Status"
	cmp	r0, #0			@ Do I need to do this?
	beq	check_uart2		@ keep waiting for a byte

@ got one; send it out to GPIO

	ldrb	r0, [r3]		@ Load in the received byte
	str	r0, [r1]		@ GPIO_OUTPUT = UART received byte

@ Send it out the UART; we can rely on that fact that the TX register is ready
@ since the rx is running at the same bit rate the tx is running at.

	strb	r0, [r4]		@ Store the TEXT byte in the transmit register

check_uart2:

@ Check for received bytes on UART2

	ldrb	r0, [r5]		@ Load the status register (also resets error flags and rx/tx ready flags)
	and	r0, r0, #0b0000001	@ Clear all bits but "Receive Register Status"
	cmp	r0, #0			@ Do I need to do this?
	beq	echo_loop		@ keep waiting for a byte

@ got one; send it out to GPIO

	ldrb	r0, [r6]		@ Load in the received byte
	str	r0, [r1]		@ GPIO_OUTPUT = UART received byte

@ Send it out the UART; we can rely on that fact that the TX register is ready
@ since the rx is running at the same bit rate the tx is running at.

	strb	r0, [r7]		@ Store the TEXT byte in the transmit register
	b	echo_loop		@ loop for ever

@ -----------------------------------------------------------------------

@	.end
