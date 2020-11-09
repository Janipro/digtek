.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start
	
Start:
	//SYSTICK_CTRL
	LDR R0, =SYSTICK_BASE + SYSTICK_CTRL
	ORR R1, R0, 0b110
	STR R1, [R0]

	//SYSTICK_LOAD
	LDR R0, =SYSTICK_BASE + SYSTICK_LOAD
	LDR R1, =FREQUENCY / 10
	STR R1, [R0]

	//SYSTICK_VAL
	LDR R0, =SYSTICK_BASE + SYSTICK_VAL
	MOV R1, #0
	STR R1, [R0]

	//EXTIPSELH
	LDR R0, =GPIO_BASE + GPIO_EXTIPSELH
	LDR R1, [R0]
	AND R1, ~(0b1111 << 4)
	ORR R1, 0b0001 << 4
	STR R1, [R0]

	//EXTIFALL
	LDR R0, =GPIO_BASE + GPIO_EXTIFALL
	LDR R1, [R0]
	ORR R1, R1, 0b1000000000
	STR R1, [R0]

	//IEN
	LDR R0, =GPIO_BASE + GPIO_IEN
	LDR R1, [R0]
	ORR R1, R1, 0b1000000000
	STR R1, [R0]

	Loop:
		B Loop

.global SysTick_Handler
.thumb_func
SysTick_Handler:
	LDR R0, =tenths
	LDR R1, [R0]
	ADD R1, R1, #1
	STR R1, [R0]
	CMP R1, #10
	BEQ Seconds
	BX LR

	Seconds:
		LDR R0, =GPIO_BASE + (LED_PORT * PORT_SIZE) + GPIO_PORT_DOUTTGL
		ORR R1, R1, 0b00000100
		STR R1, [R0]

		LDR R0, =seconds
		LDR R1, [R0]
		ADD R1, R1, #1
		STR R1, [R0]

		LDR R2, =tenths
		MOV R3, #0
		STR R3, [R2]

		CMP R1, #60
		BEQ Minutes
		BX LR

	Minutes:
		LDR R0, =minutes
		LDR R1, [R0]
		ADD R1, R1, #1
		STR R1, [R0]

		LDR R2, =seconds
		MOV R3, #0
		STR R3, [R2]
		BX LR

.global GPIO_ODD_IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:
	LDR R0, =SYSTICK_BASE + SYSTICK_CTRL
	LDR R1, [R0]
	EOR R1, R1, 0b001
	STR R1, [R0]

	LDR R0, =GPIO_BASE + GPIO_IFC
	ORR R1, R0, 0b1000000000
	STR R1, [R0]
	BX LR

NOP

