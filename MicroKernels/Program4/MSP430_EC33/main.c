/* Ejercicio EC3 */

/* Standard includes. */
#include <MSP430F5xx_6xx/driverlib.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* Hardware includes. */
#include "msp430.h"
#include "hal.h"

#include "serial.h"
#include "commands.h" //Solucion EC32: Importo commands.h para usar la misma cola que en commands.c



/* BaudRate usado por la tarea del puerto serie */
#define mainCOM_TEST_BAUD_RATE			( 9600 )

static EventGroupHandle_t FlagsEventos;

//Etiquetas de ayuda para los flags queden más legible
//puedo definir hasta 8 con la configuracion que he cogido.
#define BUTTON1_FLAG 0x0001
#define BUTTON2_FLAG 0x0002



/*-----------------------------------------------------------*/

/* declaracion de funciones */

/*
 * Funcion que configura el hardware.
 */
static void prvSetupHardware( void );

// Semáforo binario para indicar si la tarea debe detenerse
SemaphoreHandle_t xStopLEDsOnOffSemaphore;

/*
 * Declaración de tareas
 */
//static void GreenLedBlink( void *pvParameters );
static void ReadADCTask( void *pvParameters );
/*-----------------------------------------------------------*/

//Funcion main
void main( void )
{
	/* antes que nada, configuramos el micro y los perifericos */
	prvSetupHardware();

	//Inicializa la biblioteca serial.c
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE , 32 );

	//Solucion EC32: Creo una semaforo que se active al pulsar el boton derecho
	xStopLEDsOnOffSemaphore = xSemaphoreCreateBinary();
	if( xStopLEDsOnOffSemaphore == NULL )
	    {
	             while(1);
	    }

	//Inicializa el interprete de comandos
	initCommands();

    /* luego creamos las tareas */
//    if (xTaskCreate( GreenLedBlink, "GBli", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL )!=pdPASS)
//    {
//        while (1);
//    }


    /* Arranca el scheduler */
	vTaskStartScheduler();	//Ejecuta el bucle del RTOS. De aquí no se retorna...


	/* Si llegamos a este punto es que algo raro ha pasado. Ponemos un bucle infinito para depurar */
	taskDISABLE_INTERRUPTS();
	for( ;; );
	__no_operation();
}
/*-----------------------------------------------------------*/

//Esta tarea parpadea el led verde
//static void GreenLedBlink( void *pvParameters )
//{
//
//    //La tarea debe tener un bucle infinito...
//    for( ;; )
//    {
//        vTaskDelay(0.2*configTICK_RATE_HZ);
//        GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN);
//
//        vTaskDelay(0.2*configTICK_RATE_HZ); // Comentar esta y descomentar la de abajo para comprobar la diferencia de gasto de CPU
////        __delay_cycles(3200000);  //  descomentar esta línea Aproximadamente 0.2 segundos también
//
//        GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN);
//    }
//}

/*-----------------------------------------------------------*/



//Esta es la funcion que realiza la configuración inicial del hardware.
static void prvSetupHardware( void )
{
	taskDISABLE_INTERRUPTS();
	
	/* Disable the watchdog. */

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

   // Minumum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .

#ifndef DRIVERLIB_LEGACY_MODE
   PMM_setVCore(PMM_CORE_LEVEL_3);
#else

   PMM_setVCore(PMM_BASE, PMM_CORE_LEVEL_2);
#endif

    initPorts();           // Config GPIOS for low-power (output low)


    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5); //Conecta el cristal XT1LF...

    initClocks(configCPU_CLOCK_HZ);   // Config clocks. MCLK=SMCLK=FLL=configCPU_CLOCK_HZ; ACLK=XT1LF=32kHz

   	initButtons();         // Init the two buttons
}
/*-----------------------------------------------------------*/



/* esta función configura el tick del RTOS, utilizando para ello el TimerA0, que no debe utilizarse para otras cosas.*/
/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER0_A0_VECTOR. */
void vApplicationSetupTimerInterrupt( void )
{
const unsigned short usACLK_Frequency_Hz = 32768;

	/* Ensure the timer is stopped. */
	TA0CTL = 0;

	/* Run the timer from the ACLK. */
	TA0CTL = TASSEL_1;

	/* Clear everything to start with. */
	TA0CTL |= TACLR;

	/* Set the compare match value according to the tick rate we want. */
	TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

	/* Enable the interrupts. */
	TA0CCTL0 = CCIE;

	/* Start up clean. */
	TA0CTL |= TACLR;

	/* Up mode. */
	TA0CTL |= MC_1;
}
/*-----------------------------------------------------------*/

//Esta es la función "gancho" a la tarea Idle. Se puede usar para entrar en bajo consumo
void vApplicationIdleHook( void )
{
	/* Called on each iteration of the idle task.  In this case the idle task
	just enters a low(ish) power mode. */
	__bis_SR_register( LPM0_bits + GIE );
}
///*-----------------------------------------------------------*/


//Función gancho al desbordamiento de pila. El RTOS llama a a esta función si detecta dicho desbordamiento.
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;
	
	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

/*-----------------------------------------------------------*/

//Funcion diferida debe tener este prototipo,en este no caso vamos a utilizar los argumentos que se le pasan a xTimerPendFunctionCallFromISR()
void Button1_diferido(void *pvParameter1, uint32_t ulParameter2)
{
    //Esto se ejecuta en diferido
    //Dentro de esta función estamos a nivel de tarea --> podemos y debemos usar las funcione para gestionar IPC desde una tarea.
    //Las interrupciones tienen prioridad sobreesta función
    xSerialPrintf(0,"Boton 1 pulsado\r\n");

    //Solucion EC32: Funcion diferida del boton 1. Activo el semaforo para que se pare la tarea y elimine los elementos de la cola
    xSemaphoreGive(xStopLEDsOnOffSemaphore);

}

//Rutina de interrupcion del pulsador derecho (p1.1).
void __attribute__ ((interrupt(PORT1_VECTOR))) Button1_ISR (void)

{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //debe ponerse a false

    __delay_cycles(8000);	//antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
    						//Por simplicidad lo dejamos así...
    if (GPIO_getInputPinValue(BUTTON1_PORT, BUTTON1_PIN))
    {
        xTimerPendFunctionCallFromISR(Button1_diferido,(void *)&FlagsEventos,BUTTON1_FLAG,&xHigherPriorityTaskWoken);
    }

    GPIO_clearInterrupt(BUTTON1_PORT, BUTTON1_PIN);
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//Esto está un poco rebuscado intencionadamente, para demostrar la posibilidad de dividir una interrupción en dos partes

//Funcion diferida debe tener este prototipo,en este no caso vamos a utilizar los argumentos que se le pasan a xTimerPendFunctionCallFromISR()
void Button2_diferido(void *pvParameter1, uint32_t ulParameter2)
{
    typedef struct {
        int color;
        uint16_t veces;
        float frecuencia; //EC31
    } Data;

    Data data_enviar;
    //Esto se ejecuta en diferido
    //Dentro de esta función estamos a nivel de tarea --> podemos y debemos usar las funcione para gestionar IPC desde una tarea.
    //Las interrupciones tienen prioridad sobreesta función
    xSerialPrintf(0,"Boton 2 pulsado\r\n");

    //Solucion EC32: Envio el comportamiento en caso de pulsar boton izquierdo
    data_enviar.color=0; //Solucion EC32: Color rojo
    data_enviar.veces=5; //Solucion EC32: cantidad de parpadeos (numero entero) 5 veces segun el enunciado
    data_enviar.frecuencia=1; //Solucion EC31: frecuencia (en punto flotante) 1Hz segun el enunciado

    //Solucion EC32: Envio la estructura a la cola1
    xQueueSend(cola1,&data_enviar,portMAX_DELAY);
}

//Rutina de interrupcion del pulsador izquierdo (P2.1)
void __attribute__ ((interrupt(PORT2_VECTOR))) Button2_ISR (void)

{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //debe ponerse a false

    __delay_cycles(8000);	//antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
    						//Por simplicidad lo dejamos así...
    if (GPIO_getInputPinValue(BUTTON2_PORT, BUTTON2_PIN))
    {
        xTimerPendFunctionCallFromISR(Button2_diferido,(void *)&FlagsEventos,BUTTON2_FLAG,&xHigherPriorityTaskWoken);
    }

    GPIO_clearInterrupt(BUTTON2_PORT, BUTTON2_PIN);
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/* Fin ejercicio EC3 */






