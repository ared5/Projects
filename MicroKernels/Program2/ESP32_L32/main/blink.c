/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
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


void vTimerLed1Callback( TimerHandle_t pxTimer )
{
	static int estado=0;
	static int iteraciones=0;
	gpio_set_level(BLINK_GPIO_1, estado);
	estado=!estado;

	//Descomentar L32.4
//	iteraciones++;
//	if (iteraciones>=20) xTimerStop(pxTimer,0); //Me detengo a m� mismo....
}

void vTimerLed2Callback( TimerHandle_t pxTimer )
{
	static int estado=0;
	static int iteraciones=0;
	gpio_set_level(BLINK_GPIO_2, estado);
	estado=!estado;

	//Descomentar L32.4
//	iteraciones++;
//	if (iteraciones>=20) xTimerStop(timerhandle3,0); //Detengo al timer 3.
}

void vTimerLed3Callback( TimerHandle_t pxTimer )
{
	static int estado=0;
	gpio_set_level(BLINK_GPIO_3, estado);
	estado=!estado;
}



void app_main(void)
{
	int cuenta=0;

    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
	gpio_reset_pin(BLINK_GPIO_1);
	gpio_reset_pin(BLINK_GPIO_2);
	gpio_reset_pin(BLINK_GPIO_3);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_3, GPIO_MODE_OUTPUT);

    //Descomentar L32.3
//    timerhandle1 = xTimerCreate("Tmr1",0.8*configTICK_RATE_HZ,pdTRUE, ( void * ) 1, vTimerLed1Callback);
//   	 if (timerhandle1==NULL)
//   		 printf("No pude crear el timer 1");
//   	 timerhandle2 = xTimerCreate("Tmr2",1.5*configTICK_RATE_HZ,pdTRUE, ( void * ) 2, vTimerLed2Callback);
//   	 if (timerhandle2==NULL)
//   		printf("No pude crear el timer 2");
//   	timerhandle3 = xTimerCreate("Tmr3",3.5*configTICK_RATE_HZ,pdTRUE, ( void * ) 2, vTimerLed3Callback);
//   	 if (timerhandle3==NULL)
//   		printf("No pude crear el timer 3");


    while(1) {
    	//Parpadea el LED rojo
       vTaskDelay(configTICK_RATE_HZ);
       printf("Hola, soy la tarea mainTask\n");
       cuenta++;
       //Descomentar L32.3
       if (5==cuenta)
       {
    	   //Descomentar L32.3
//    	   printf("Pongo en marcha los timers \n");
//    	   xTimerStart(timerhandle1,portMAX_DELAY);
//    	   xTimerStart(timerhandle2,portMAX_DELAY);
//    	   xTimerStart(timerhandle3,portMAX_DELAY);
       }
    }
}
