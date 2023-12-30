/* Ejemplo inicial Introduccion a FreeRTOS */


/* Standard includes. */
#include <stdio.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* Hardware includes. */
#include "msp430.h"
#include "driverlib.h"
#include "hal.h"

#include "serial.h"

/* BaudRate usado por la tarea del puerto serie */
#define mainCOM_TEST_BAUD_RATE          ( 9600 )

/*-----------------------------------------------------------*/

/* declaracion de funciones */

/*
 * Funcion que configura el hardware.
 */
static void prvSetupHardware( void );


/*
 * Declaración de tareas
 */
static void GreenLedBlink( void *pvParameters );
static void RedLedBlink( void *pvParameters );
static void prvSerialTask( void *pvParameters );

/*-----------------------------------------------------------*/

//Globales
static SemaphoreHandle_t semaforo1;
static SemaphoreHandle_t semaforo2; // EC22: Añadir un nuevo semaforo para la pulsacion del pulsador P2.1

/*-----------------------------------------------------------*/

//Funcion main
void main( void )
{
	/* antes que nada, configuramos el micro y los perifericos */
	prvSetupHardware();

    //Inicializa la biblioteca serial.c
    xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE , 32 );

    if (xTaskCreate( prvSerialTask, "Ser", 100, NULL, tskIDLE_PRIORITY+1, NULL )!=pdPASS)

        while (1);

    if (xTaskCreate( RedLedBlink, "RBli", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2,
                     NULL )!=pdPASS)
        while (1);

    if (xTaskCreate( GreenLedBlink, "GBli", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1,
                     NULL )!=pdPASS)
        while (1);

    // EC22: Creacion de semaforo binario para LED rojo
//    semaforo1 = xSemaphoreCreateBinary();

    // EC23: Creacion de semaforo contador
    semaforo1 = xSemaphoreCreateCounting(10, 0);

    if( semaforo1 == NULL )
    {
        while(1);
    }

    // EC22: Creamos un semaforo binario para el LED verde
//    semaforo2 = xSemaphoreCreateBinary();

    // EC23: Creacion de semaforo contador
    semaforo2 = xSemaphoreCreateCounting(10, 0);
    if( semaforo2 == NULL )
    {
        while(1);
    }

    /* Arranca el scheduler */
    vTaskStartScheduler();	//Ejecuta el bucle del RTOS. De aquí no se retorna...


    /* Si llegamos a este punto es que algo raro ha pasado. Ponemos un bucle infinito para depurar */
    taskDISABLE_INTERRUPTS();
    for( ;; );
	__no_operation();
}
/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/
//Esta tarea parpadea el led verde
static void GreenLedBlink( void *pvParameters )

{
    uint8_t contador = 0; // EC27: Añadimos variable contador para llevar cuenta de cuantas veces encendido LED verde
    uint8_t contadorDeleteTask = 0; // EC29: Crear contador para saber cuando se han hecho 8 iteraciones para despues eliminar la tarea
    //La tarea debe tener un bucle infinito...
    for( ;; )
    {
        //        xSemaphoreTake(semaforo2,portMAX_DELAY); // EC22: Cogemos el semaforo2 y se espera infinito hasta que la interrupcion lo active
        //                                                // Mintras tanto, se queda en Waiting

        // EC28: Añadimos esta parte para que en caso de que pasen 6 segundos haga la parte else (mostrar TIMEOUT)
        // y si se pulsa hace la parte de encender el LED verde
        if (xSemaphoreTake(semaforo2,6*configTICK_RATE_HZ)) {
            GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN); //EC22: Enciende el LED verde durante 1.5seg y luego lo apaga durante 1.5 seg
            vTaskDelay(1.5*configTICK_RATE_HZ);
            GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN);
            vTaskDelay(1.5*configTICK_RATE_HZ);

            contador++;
            if (contador==4) {
                contador = 0; // EC27: Reseteamos contador
                xSemaphoreGive(semaforo1); // EC27: Activamos semaforo1 del LED rojo
            }
            contadorDeleteTask++; // EC29: Cada vez que se enciende el LED verde, suma en el contador para eliminar la tarea al llegar a 8
        }
        else {
            xSerialPrintf(0, "TIMEOUT\r\n");
        }

        // EC29: Elimina la tarea en caso de que se haya encendido 8 veces el LED verde
        if (contadorDeleteTask == 8) {
            vTaskDelete(NULL);
        }
    }
}
/*-----------------------------------------------------------*/

//Esta tarea parpadea el led rojo
static void RedLedBlink( void *pvParameters )
{
    //La tarea debe tener un bucle infinito...
    for( ;; )
    {
        xSemaphoreTake(semaforo1,portMAX_DELAY);
        vTaskDelay(0.2*configTICK_RATE_HZ);
        GPIO_toggleOutputOnPin(LED1_PORT,LED1_PIN);
        vTaskDelay(0.2*configTICK_RATE_HZ);
        GPIO_toggleOutputOnPin(LED1_PORT,LED1_PIN);
    }
}
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

static void prvSerialTask( void *pvParameters )
{
    unsigned int i=0;

    for( ;; )
    {
        xSerialPrintf( 0,"Hola, soy la tarea serie %d\r\n",(uint32_t)i);
        i++;
        vTaskDelay(2*configTICK_RATE_HZ);
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Button1_ISR (void)

{
    BaseType_t xHigherPriorityTaskWoken=pdFALSE;

    __delay_cycles(8000);   //antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
                            //Por simplicidad lo dejamos así...

    if (GPIO_getInputPinValue(BUTTON1_PORT, BUTTON1_PIN))
    {
            xSemaphoreGiveFromISR( semaforo1, &xHigherPriorityTaskWoken );
    }

    GPIO_clearInterrupt(BUTTON1_PORT, BUTTON1_PIN);
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

// EC22: Incluimos funcion por interrupcion del pulsador P2.1
#pragma vector=PORT2_VECTOR
__interrupt void Button2_ISR (void)

{
    BaseType_t xHigherPriorityTaskWoken=pdFALSE;

    __delay_cycles(8000);   //antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
                            //Por simplicidad lo dejamos así...

    if (GPIO_getInputPinValue(BUTTON2_PORT, BUTTON2_PIN)) // EC22: Si se ha pulsado el boton P2.1
    {
        xSemaphoreGiveFromISR( semaforo2, &xHigherPriorityTaskWoken ); // EC22: Activa semaforo2 por ISR
    }

    GPIO_clearInterrupt(BUTTON2_PORT, BUTTON2_PIN); // EC22: Limpiar interrupcion del boton P2.1
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


