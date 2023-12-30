/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"

//Descomentar L32.3
//static TimerHandle_t timerhandle1,timerhandle2,timerhandle3; //manejadores de los timers

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
 */
#define BLINK_GPIO_1 0
#define BLINK_GPIO_2 2
#define BLINK_GPIO_3 4

/*-----------------------------------------------------------*/

static const char *TAG = "_SERIE";
static QueueHandle_t uart0_queue;

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE 1024
#define RD_BUF_SIZE BUF_SIZE

static void uart_event_task(void *pvParameters)
{
	uart_event_t event;
	size_t buffered_size;
	uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
	for(;;) {

		//Espera a que se produzca un evento (nos llega la informaci�n a trav�s de la cola de mensaje)
		if(xQueueReceive(uart0_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
			bzero(dtmp, RD_BUF_SIZE);
			ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
			switch(event.type) {
			//Event of UART receving data
			/*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
			case UART_DATA:
				//Si son datos que han llegado, imprime informacion sobre ellos
				ESP_LOGI(TAG, "[UART DATA]: Han llegado %d bytes ", event.size);
				uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
				ESP_LOGI(TAG, "[DATA EVT]: Esto es lo que ha llegado: ");
				uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
				break;
				//Event of HW FIFO overflow detected
			case UART_FIFO_OVF:
				ESP_LOGI(TAG, "hw fifo overflow");
				// If fifo overflow happened, you should consider adding flow control for your application.
				// The ISR has already reset the rx FIFO,
				// As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush_input(EX_UART_NUM);
				xQueueReset(uart0_queue);
				break;
				//Event of UART ring buffer full
			case UART_BUFFER_FULL:
				ESP_LOGI(TAG, "ring buffer full");
				// If buffer full happened, you should consider encreasing your buffer size
				// As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush_input(EX_UART_NUM);
				xQueueReset(uart0_queue);
				break;
				//Event of UART RX break detected
			case UART_BREAK:
				ESP_LOGI(TAG, "uart rx break");
				break;
				//Event of UART parity check error
			case UART_PARITY_ERR:
				ESP_LOGI(TAG, "uart parity error");
				break;
				//Event of UART frame error
			case UART_FRAME_ERR:
				ESP_LOGI(TAG, "uart frame error");
				break;
				//UART_PATTERN_DET
			case UART_PATTERN_DET:
				//Caso patron detectado....
				uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
				int pos = uart_pattern_pop_pos(EX_UART_NUM);
				ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
				if (pos == -1) {
					// There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
					// record the position. We should set a larger queue size.
					// As an example, we directly flush the rx buffer here.
					uart_flush_input(EX_UART_NUM);
				} else {
					uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
					uint8_t pat[PATTERN_CHR_NUM + 1];
					memset(pat, 0, sizeof(pat));
					uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
					ESP_LOGI(TAG, "read data: %s", dtmp);
					ESP_LOGI(TAG, "read pat : %s", pat);
				}
				break;
				//Others
			default:
				ESP_LOGI(TAG, "uart event type: %d", event.type);
				break;
			}
		}
	}
	free(dtmp);
	dtmp = NULL;
	vTaskDelete(NULL);
}



void app_main(void)
{
	int cuenta=0;

	/* Configura los pines GPIO  de los LEDs como salida
	 */
	gpio_reset_pin(BLINK_GPIO_1);
	gpio_reset_pin(BLINK_GPIO_2);
	gpio_reset_pin(BLINK_GPIO_3);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
	gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);
	gpio_set_direction(BLINK_GPIO_3, GPIO_MODE_OUTPUT);

	/* configura la UART */
	esp_log_level_set(TAG, ESP_LOG_INFO);
	/* Configure parameters of an UART driver,
	 * communication pins and install the driver */
	uart_config_t uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(EX_UART_NUM, &uart_config);

	//Set UART log level
	esp_log_level_set(TAG, ESP_LOG_INFO);
	//Set UART pins (using UART0 default pins ie no changes.)
	uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//Install UART driver, and get the queue.
	uart_driver_install(EX_UART_NUM, BUF_SIZE , BUF_SIZE , 20, &uart0_queue, 0);

	//Establece la detecci�n del patr�n "+++"
	uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 10000, 10, 10);
	//Reset the pattern queue length to record at most 20 pattern positions.
	uart_pattern_queue_reset(EX_UART_NUM, 20);

	//Crea ima tarea para gestionar los eventos de la UART:
	xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

	while(1) {
		//Parpadea el LED rojo
		gpio_set_level(BLINK_GPIO_1, 0);
		vTaskDelay(configTICK_RATE_HZ);
		gpio_set_level(BLINK_GPIO_1, 1);
		vTaskDelay(configTICK_RATE_HZ);
	}
}
