.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start
	
Start:

	Loop:
	LDR R2, =GPIO_BASE + (BUTTON_PORT * PORT_SIZE) + GPIO_PORT_DIN
	LDR R3, [R2]
	AND R4, R3, 0b1000000000
	CMP R4, 0b1000000000
	BNE Lys

	IkkeLys:
	LDR R2, =GPIO_BASE + (LED_PORT * PORT_SIZE) + GPIO_PORT_DOUTCLR
	ORR R3, R2, 0b00000100
	STR R3, [R2]
	B Loop

	Lys:
	LDR R0, =GPIO_BASE + (LED_PORT * PORT_SIZE) + GPIO_PORT_DOUTSET
	ORR R1, R0, 0b00000100
	STR R1, [R0]
	B Loop








NOP // Behold denne på bunnen av fila

