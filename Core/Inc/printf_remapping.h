
#ifndef ___PRINTF___REMAPPING__H___
#define ___PRINTF___REMAPPING__H___

#define cli_buffer_maxlen 1024
extern uint8_t cli_line_data[cli_buffer_maxlen];
extern uint8_t cli_line_datalen;
extern uint8_t cli_enter_avilable;
extern uint8_t cli_line_available;


#define PRINTF_REMAPPING_USE_USB_VCOM 0
#define PRINTF_REMAPPING_USE_USART 1

extern int __custom_printf__(const char *format, ...);

void printf_remapping_init();
void printf_remapping_process();

#endif
