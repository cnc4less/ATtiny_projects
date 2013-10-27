#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware_conf.h"
#include "system_ticker.h"
#include "main.h"

#if !defined(NO_MOSFET) && !defined(USING_MOSFET)
#error "Something wrong with 'hardware_conf.h' !"
#endif

uint8_t EEMEM saved_mode = 0;

int main(void)
{
	uint8_t mode = eeprom_read_byte(&saved_mode);

	setup_hw();

	if (PB0_PB2_shorted()) {	// ISP header pin #3 and #4 shorted on power-up
		delay(20000); // requires system-ticker ISR to run!
		if (PB0_PB2_shorted()) {	// still shorted
			mode++;
			if(mode == 10) {	// cycle 0..1..2..3..4..5..6..7..8..9..0..1..2...
				mode = 0;
			}
			eeprom_write_byte(&saved_mode, mode);
		}
	}

	setup_hw();		// set and/or reset verything we need for normal operation

	switch (mode) {
	case 0:
		brightness_a = 255;
		while(1) {}
		break;
	case 1:
		brightness_b = 255;
		while(1) {}
		break;
	case 2:
		brightness_a = 255;
		brightness_b = 255;
		while(1) {}
		break;
	case 3:
		breathe(75, 0, 0);	// delay, color, times (0 --> infinite loop)
		break;
	case 4:
		breathe(75, 1, 0);
		break;
	case 5:
		breathe(75, 2, 0);
		break;
	case 6:
	    while(1) {	
			burst(5,5000,5,500,0);
		}
		break;
	case 7:
	    while(1) {	
			burst(5,5000,5,500,1);
		}
		break;
	case 8:
	    while(1) {	
			burst(5,5000,5,500,2);
		}
		break;
	case 9:
		while(1) {
			rainbow(100);
		}
		break;
	default:
		break;
	}
}

void setup_hw(void)
{
	cli();			// turn interrupts off, just in case

	DDRB = 0x00;		// set all as input
	PORTB = 0xFF;		// set all pull-ups on

	system_ticker_setup();

	sei();			// turn global irq flag on
}

void fade(uint8_t from, uint8_t to, uint16_t f_delay, uint8_t color)
{
	int16_t counter;

	if (from <= to) {	// fade up 
		for (counter = from; counter <= to; counter++) {
			helper(color, counter); // save space by avoiding code duplication
			delay(f_delay);
		}
	}

	if (from > to) {	// fade down 
		for (counter = from; counter >= to; counter--) {
			helper(color, counter); // save space by avoiding code duplication
			delay(f_delay);
		}
	}
}

void helper(uint8_t color, uint8_t value) {
			if (color == 0) {
				brightness_a = value;
			}
			if (color == 1) {
				brightness_b = value;
			}
			if (color == 2) {
				brightness_a = value;
				brightness_b = value;
			}
}


void breathe(uint16_t b_delay, uint8_t color, int8_t times)
{
	if (times == 0) {
		while (1) {
			fade(0, 255, b_delay, color);	// from, to, delay, color
			fade(255, 0, b_delay, color);
		}
	} else {
		while (times > 0) {

			fade(0, 255, b_delay, color);
			fade(255, 0, b_delay, color);
			times--;
		}

	}
}

void burst(uint8_t bursts, uint16_t burst_delay, uint8_t pulses, uint16_t pulse_delay, uint8_t color)
{
	uint8_t pulse_ctr;
	uint8_t burst_ctr;
	
	for(burst_ctr = bursts ; burst_ctr > 0; burst_ctr--) {
		for(pulse_ctr = pulses; pulse_ctr > 0; pulse_ctr--) {
			if (color == 0) {
				brightness_a = 255;
				delay(pulse_delay),
				brightness_a = 0;
				delay(pulse_delay);
			}
			if (color == 1) {
				brightness_b = 255;
				delay(pulse_delay);
				brightness_b = 0;
				delay(pulse_delay);
			}
			if (color == 2) {
				brightness_a = 255;
				brightness_b = 255;
				delay(pulse_delay);
				brightness_a = 0;
				brightness_b = 0;
				delay(pulse_delay);
			}
		}
		delay(burst_delay);
	}
}

void rainbow(uint16_t rainbow_delay) {
	int16_t ctr;

	for(ctr = 0; ctr <= 255; ctr++) {
		brightness_a = ctr;	// color a UP
		brightness_b = 255 - ctr; // color b DOWN
		delay(rainbow_delay);	
	}

	for(ctr = 0; ctr <= 255; ctr++) {
		brightness_b = ctr; // color b UP
		delay(rainbow_delay);	
	}

	for(ctr = 255; ctr > 0; ctr--) {
		brightness_a = ctr; // color a DOWN
		delay(rainbow_delay);	
	}
}

uint8_t PB0_PB2_shorted(void)
{
	uint8_t retval = 0;
	DDRB |= _BV(PB0);	// set PB0 as output
	PORTB &= ~_BV(PB0);	// set PB0 to LOW

	DDRB &= ~_BV(PB2);	// set PB2 as input
	PORTB |= _BV(PB2);	// pull-up on on PB2

	if (!(PINB & _BV(PB2))) {	// PB2 reads as LOW
		retval = 1;
	} else {
		retval = 0;
	}

	// don't reset pins PB0, PB2 to inputs here
	// setup_hw() will be called later in main()
	// which does all of that 

	return retval;
}