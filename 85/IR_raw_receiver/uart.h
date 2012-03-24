#ifndef uart_h
#define uart_h

// 38400 values (reduced a bit, running on 16MHz xtal for testing)
// may not work with ceramic resonator
#define HALF_BIT_DELAY         11 // tuned a bit after looking at the sample times
#define FULL_BIT_DELAY         23 // with my newly acquired OLS from Dangerous Prototypes ;-)
#define THREE_HALFS_BIT_DELAY  35

void soft_uart_send(uint8_t byte);
void soft_uart_send_byte_number(uint8_t number);
void soft_uart_send_uint_number(uint16_t number);
void soft_uart_send_PSTR(const char * string);

#endif
