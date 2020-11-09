#include "o3.h"
#include "gpio.h"
#include "systick.h"

/**************************************************************************//**
 * @brief Konverterer nummer til string 
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;
		
	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}

int hours = 0;
int minutes = 0;
int seconds = 0;
int state = SET_SEC;

//Lager et memory map ved bruk av peker
volatile gpio_map_t* gpio_map = (gpio_map_t*) GPIO_BASE;
volatile systick_t* systick_map = (systick_t*) SYSTICK_BASE;

void setup_I_O() {
	volatile word mask = gpio_map->ports[B0_PORT].MODEH;

	//Kobler LED til output
	gpio_map->ports[LED_PORT].DOUT &= 1 << LED_PIN;
	gpio_map->ports[LED_PORT].MODEL &= 0b0100 << 8;
	gpio_map->ports[LED_PORT].MODEL |= 0b0100 << 8;

	//S� buttons til input
	mask &= ~(0b1111 << 4);
	mask |= 0b0001 << 4;

	mask &= ~(0b1111 << 8);
	mask |= 0b0001 << 8;
	gpio_map->ports[B1_PORT].MODEH = mask;
}

void initialize() {
	setup_I_O();

	//Interrupt
	gpio_map->EXTIPSELH &= ~(0b1111 << 4);
	gpio_map->EXTIPSELH |= 0b0001 << 4;
	gpio_map->EXTIPSELH &= ~(0b1111 << 8);
	gpio_map->EXTIPSELH |= 0b0001 << 8;

	gpio_map->EXTIFALL |= 1 << B0_PIN;
	gpio_map->EXTIFALL |= 1 << B1_PIN;

	gpio_map->IEN |= 1 << B0_PIN;
	gpio_map->IEN |= 1 << B1_PIN;

	//Systick
	systick_map->CTRL |= 0b110;
	systick_map->LOAD = FREQUENCY;
}

void update_lcd() {
	time_to_string(str, hours, minutes, seconds);
	lcd_write(str);
}

void start_count() {
	systick_map->VAL = systick_map->LOAD;
	systick_map->CTRL |= 0b001;
}

void stop_count() {
	systick_map->CTRL &= 0b110;
}

void increment_hours() {
	hours ++;
}

void increment_minutes() {
	minutes ++;
	if (minutes == 60) {
		minutes = 0;
		increment_hours();
	}
}

void increment_seconds() {
	seconds ++;
	if (seconds == 60) {
		seconds = 0;
		increment_minutes();
	}
}

void GPIO_ODD_IRQHandler() {
	switch(state) {
		case SET_SEC: {
			increment_seconds();
			update_lcd();
		} break;
		case SET_MIN: {
			increment_minutes();
			update_lcd();
		} break;
		case SET_HOUR: {
			increment_hours();
			update_lcd();
		} break;
		case COUNT_DOWN: break;
		case ALARM: {
			gpio_map->ports[LED_PORT].DOUTSET = 1 << LED_PIN;
		}
	}

	gpio_map->IFC |= 1 << B0_PIN;
}

void GPIO_EVEN_IRQHandler() {
	if (state == SET_HOUR) {
		start_count();
	}

	if (state == COUNT_DOWN) {
		gpio_map->IFC |= 1 << B1_PIN;
		return;
	}

	if (state == ALARM) {
		state = SET_SEC;
		gpio_map->ports[LED_PORT].DOUTTGL = 1 << LED_PIN;
		gpio_map->IFC |= (1 << B1_PIN);
		return;
	}

	state ++;
	gpio_map->IFC |= 1 << B1_PIN;
}

void SysTick_Handler() {
	if (state == COUNT_DOWN) {
		seconds --;
		update_lcd();
		if (seconds <= 0) {
			minutes --;
			seconds = 59;
			update_lcd();
		}

		if (minutes == -1) {
			hours --;
			minutes = 59;
			update_lcd();
		}

		if (hours == -1) {
			stop_count();
			state = ALARM;
			hours = 0;
			minutes = 0;
			seconds = 0;
			gpio_map->ports[LED_PORT].DOUTSET = 1 << LED_PIN;
			update_lcd();
		}
	}
}

int main(void) {
    init();
    initialize();
    update_lcd();
    while (1);
}
