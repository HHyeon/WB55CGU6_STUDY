
#include "main.h"
#include "printf_remapping.h"
#include "stdio.h"
#include "stdarg.h"

#include "usbd_cdc_if.h"
#include "list_object.h"


#if PRINTF_REMAPPING_USE_USB_VCOM
extern USBD_HandleTypeDef hUsbDeviceFS;
#define logging_cdc_instance hUsbDeviceFS
#endif

#if PRINTF_REMAPPING_USE_USART
extern UART_HandleTypeDef huart1;
#define logging_uart_instance huart1
#endif

uint8_t cli_line_data[cli_buffer_maxlen];
uint8_t cli_line_datalen = 0;
uint8_t cli_enter_avilable = 0;
uint8_t cli_line_available = 0;

void cli_buffer_queue_object_print(void* p) {}

DEFINE_WTS_QUEUEOBJECT(cli_buffer_queue_object, uint8_t, 2048, cli_buffer_queue_object_print);

void printf_mapping(int ch)
{
  wts_Queue_enqueue(&cli_buffer_queue_object, (uint8_t*)&ch);
}

const uint8_t cr_char = '\r';
int __io_putchar(int ch)
{
  if(ch == '\n')
  {
    printf_mapping(cr_char);
  }

  printf_mapping(ch);

  return ch;
}

char buffer_log_printf_arguments[256];
va_list args_copy;
// Function to log the formatted string with arguments
void log_printf_arguments(const char *format, va_list args) {

    // Copy the argument list to use it for vsnprintf
    va_copy(args_copy, args);

    uint16_t len=0;

    if(format[0] != '\n')
    {
      // Format the string into the buffer
      sprintf(buffer_log_printf_arguments, "%05ld: ", HAL_GetTick()%100000);
      len = strlen(buffer_log_printf_arguments);
    }

    vsnprintf(buffer_log_printf_arguments+len, sizeof(buffer_log_printf_arguments)-len, format, args_copy);

    // Print the log message (this could be changed to write to a file or another logging mechanism)
    for (char *p = buffer_log_printf_arguments; *p != '\0'; p++) {
      __io_putchar(*p);
    }

    // Clean up the copied argument list
    va_end(args_copy);
}

uint16_t cli_buffer_queue_object_counts = 0;

int __custom_printf__(const char *format, ...)
{
    va_list args;
//    int ret;

    // Lock the mutex before calling printf
//    osMutexAcquire(printfMutexHandle, osWaitForever);

    // Initialize the variable argument list
    va_start(args, format);

    log_printf_arguments(format, args);
//    vprintf(format, args);


    // Clean up the variable argument list
    va_end(args);

    // Unlock the mutex after calling printf
//    osMutexRelease(printfMutexHandle);

    cli_buffer_queue_object_counts = wts_Queue_count(&cli_buffer_queue_object);

    return 0;
}

void cli_single_char_process(uint8_t rxchar, uint8_t *buffer, uint16_t buffersize, uint8_t *bufferindex)
{

  if(rxchar == '\r')
  {
    if(*bufferindex == 0)
    {
      cli_enter_avilable = 1;
    }
    else
    {
      cli_line_datalen = *bufferindex;
      memcpy(cli_line_data, buffer, cli_line_datalen);
      cli_line_data[cli_line_datalen] = '\0';

      cli_line_available = 1;
    }

    *bufferindex = 0;
    memset(buffer, 0, buffersize);
  }
  else if(' ' <= rxchar && rxchar <= '~')
  {
    if(*bufferindex < cli_buffer_maxlen-1)
    {
      buffer[(*bufferindex)++] = rxchar;
    }
  }
}


uint8_t cdc_input_buffer[cli_buffer_maxlen];
uint8_t cdc_input_buffer_index = 0;


#if PRINTF_REMAPPING_USE_USART

uint8_t cli_uart_rx_char = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(logging_uart_instance.Instance == huart->Instance)
  {
    cli_single_char_process(cli_uart_rx_char, cdc_input_buffer, sizeof(cdc_input_buffer), &cdc_input_buffer_index);

    HAL_UART_Receive_IT(&huart1, &cli_uart_rx_char, 1);
  }
}

volatile uint8_t uart_tx_done = 1;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(logging_uart_instance.Instance == huart->Instance)
  {
    uart_tx_done = 1;
  }
}

#endif

#if PRINTF_REMAPPING_USE_USB_VCOM

void user_cdc_rx_buffer_process(uint8_t *data, uint32_t len)
{
  uint32_t i=0;

  for(i=0;i<len;i++)
  {
    if(data[i] == '\0') break;

    cli_single_char_process(data[i], cdc_input_buffer, sizeof(cdc_input_buffer), &cdc_input_buffer_index);

  }
}
#endif





void printf_remapping_init()
{
  wts_Queue_Reset(&cli_buffer_queue_object);

#if PRINTF_REMAPPING_USE_USART
  HAL_UART_Receive_IT(&huart1, &cli_uart_rx_char, 1);
#endif
}

uint8_t vcom_comm_enable = 0;
uint8_t vcom_devstate_past = 0;
uint32_t vcom_comm_state_configured_set_delay_timer_enable = 0;
uint32_t vcom_comm_state_configured_set_delay_tickstore = 0;

#if PRINTF_REMAPPING_USE_USART
char uart_tx_buffer[50] = {0,};
#endif

char printf_lineout_buffer[50] = {0,};
char printf_lineout_buffer_cat[2];

uint32_t printf_remapping_process_interval_tickstore=0;
void printf_remapping_process()
{

#if PRINTF_REMAPPING_USE_USB_VCOM

  static uint32_t vcom_devstate_interval_check_tickstore = 0;
  if(vcom_devstate_interval_check_tickstore + 100 <= HAL_GetTick())
  {
    vcom_devstate_interval_check_tickstore = HAL_GetTick();

    if(vcom_devstate_past != logging_cdc_instance.dev_state)
    {
      if(logging_cdc_instance.dev_state == USBD_STATE_CONFIGURED)
      {
        vcom_comm_state_configured_set_delay_timer_enable = 1;
        vcom_comm_state_configured_set_delay_tickstore = HAL_GetTick();
      }
      else
      {
        vcom_comm_enable = 0;
      }
    }
    vcom_devstate_past = logging_cdc_instance.dev_state;

    if(vcom_comm_state_configured_set_delay_timer_enable)
    {
      if(vcom_comm_state_configured_set_delay_tickstore + 500 <= HAL_GetTick())
      {
        vcom_comm_state_configured_set_delay_timer_enable = 0;
        vcom_comm_enable = 1;
      }
    }
  }
#endif



  if(printf_remapping_process_interval_tickstore + 10 <= HAL_GetTick())
  {
    printf_remapping_process_interval_tickstore = HAL_GetTick();

#if PRINTF_REMAPPING_USE_USB_VCOM
    if(vcom_comm_enable)
#endif

#if PRINTF_REMAPPING_USE_USART
    if(uart_tx_done == 1)
#endif
    while(wts_Queue_count(&cli_buffer_queue_object) > 0)
    {

      if(wts_Queue_accessByIndex(&cli_buffer_queue_object, 0, &printf_lineout_buffer_cat[0]))
      {
        uint8_t printout = 0;

        if(strlen(printf_lineout_buffer) <= sizeof(printf_lineout_buffer) - 2)
        {
          printf_lineout_buffer_cat[1] = '\0';
          strcat(printf_lineout_buffer, printf_lineout_buffer_cat);
          wts_Queue_dequeue(&cli_buffer_queue_object, NULL);

          if(strlen(printf_lineout_buffer) == sizeof(printf_lineout_buffer)-1) printout = 1;
        }
        else
        {
          printout = 1; // buffer full
        }

        if(printf_lineout_buffer_cat[0] == '\n')
          printout = 1;

        if(printout)
        {
#if PRINTF_REMAPPING_USE_USB_VCOM
          CDC_Transmit_FS((uint8_t*)printf_lineout_buffer, strlen(printf_lineout_buffer));
#endif

#if PRINTF_REMAPPING_USE_USART
          uart_tx_done = 0;
//          HAL_UART_Transmit(&logging_uart_instance, (uint8_t*)printf_lineout_buffer, strlen(printf_lineout_buffer), HAL_MAX_DELAY);

          memcpy(uart_tx_buffer, printf_lineout_buffer, sizeof(printf_lineout_buffer));
          HAL_UART_Transmit_IT(&huart1, (uint8_t*)uart_tx_buffer, strlen(uart_tx_buffer));
#endif
          memset(printf_lineout_buffer, 0, sizeof(printf_lineout_buffer));

          break;
        }
      }
    }

    cli_buffer_queue_object_counts = wts_Queue_count(&cli_buffer_queue_object);

  }
}
















