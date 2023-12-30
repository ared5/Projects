//*****************************************************************************
//
// Fichero con la definicion de comandos del interprete
//
//
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#include <stdio.h>
#include <assert.h>
#include <MSP430F5xx_6xx/driverlib.h>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "utils/cmdline.h"

#include "serial.h"

#include "msp430.h"
#include "hal.h"

#include "commands.h" //Solucion EC31: Importo del header para usar la cola1

QueueHandle_t cola1; //Solucion EC31: Definición de la cola1
static SemaphoreHandle_t xCancelLEDsOnOffSemaphore; //Solucion EC33: Defino el semaforo para la tarea Cmd_cancel


// ==============================================================================
// Implementa el comando free, que muestra cuánta memoria "heap" le queda al FreeRTOS
// ==============================================================================
int Cmd_free(int argc, char *argv[])
{
    //
    // Print some header text.
    //
	xSerialPrintf(0,"%d bytes libres\r\n",(uint32_t) xPortGetFreeHeapSize());

    // Return success.
    return(0);
}

// ==============================================================================
// Implementa el comando task. Sólo es posible si la opción configUSE_TRACE_FACILITY de FreeRTOS está habilitada
// ==============================================================================
#if ( configUSE_TRACE_FACILITY == 1 )

extern char *__stack;
static int Cmd_tasks(int argc, char *argv[])
{
	char*	pBuffer;
	unsigned char*	pStack;
	portBASE_TYPE	x;
	
	pBuffer = pvPortMalloc(1024);
	if (pBuffer)
	{
		vTaskList(pBuffer);
		xSerialPrintf(0,"\t\t\t\tUnused\r\nTaskName\tStatus\tPri\tStack\tTask ID\r\n");
		xSerialPrintf(0,"=======================================================\r\n");
		xSerialPrintf(0,"%s", pBuffer);
		vPortFree(pBuffer);
	}
	return 0;
}

#endif /* configUSE_TRACE_FACILITY */

// ==============================================================================
// Implementa el comando help
// ==============================================================================
static int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *pEntry;

    //
    // Print some header text.
    //
    xSerialPrintf(0,"Comandos disponibles\r\n");
    xSerialPrintf(0,"------------------\r\n");

    //
    // Point at the beginning of the command table.
    //
    pEntry = &g_psCmdTable[0];

    //
    // Enter a loop to read each entry from the command table.  The end of the
    // table has been reached when the command name is NULL.
    //
    while(pEntry->pcCmd)
    {
        //
        // Print the command name and the brief description.
        //
    	xSerialPrintf(0,"%s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);

        //
        // Advance to the next entry in the table.
        //
        pEntry++;
    }

    //
    // Return success.
    //
    return(0);
}

// ==============================================================================
// Implementa el comando "LED"
// ==============================================================================
static int Cmd_led(int argc, char *argv[])
{

// COMANDO LED COMPLETO (INICIO)
    if (argc != 3)
    {
        //Si los parámetros no son suficientes, muestro la ayuda
        xSerialPrintf(0," LED [red|green] [on|off]\r\n");
    }
    else
    {
        /* chequeo el parametro */
        if (0==strncmp( argv[1], "red",3))
        {

                if (0==strncmp( argv[2], "on",2))
                {
                    xSerialPrintf(0,"Enciendo el LED rojo\r\n");
                    GPIO_setOutputHighOnPin(LED1_PORT,LED1_PIN);
                }
                else if (0==strncmp( argv[2], "off",3))
                {
                    xSerialPrintf(0,"Apago el LED rojo\r\n");
                    GPIO_setOutputLowOnPin(LED1_PORT,LED1_PIN);
                }
                else
                {
                    //Si el parámetro no es correcto, muestro la ayuda
                    xSerialPrintf(0," LED [red|green] [on|off]\r\n");
                }

        }
        else if (0==strncmp( argv[1], "green",5))
        {

                if (0==strncmp( argv[2], "on",2))
                {
                    xSerialPrintf(0,"Enciendo el LED verde\r\n");
                    GPIO_setOutputHighOnPin(LED2_PORT,LED2_PIN);
                }
                else if (0==strncmp( argv[2], "off",3))
                {
                    xSerialPrintf(0,"Apago el LED verde\r\n");
                    GPIO_setOutputLowOnPin(LED2_PORT,LED2_PIN);
                }
                else
                {
                    //Si el parámetro no es correcto, muestro la ayuda
                    xSerialPrintf(0," LED [red|green] [on|off]\r\n");
                }
        }
        else xSerialPrintf(0," LED [red|green] [on|off]\r\n");

    }
    return 0;
}
// COMANDO LED COMPLETO (FIN)


// ==============================================================================
// Implementa el comando "Blink" (EC31: ¡¡¡completar!!!)
// ==============================================================================
static int Cmd_blink(int argc, char *argv[])
{
    //Solucion EC31: Creo una tipo de dato en formato estructura llamada Data
    typedef struct {
        int color;
        uint16_t veces;
        float frecuencia; //EC31
    } Data;

    //Solucion EC31:Creo una variable data de tipo Data
    Data data;

    //Solucion EC31
    if (argc != 4) //Solucion EC31: 4 argumentos de entrada, sino, se muestra mensaje
    {
        //Solucion EC31: Si los parámetros no son suficientes, muestro la ayuda
        xSerialPrintf(0," blink [red|green] [veces] [frecuencia]\r\n"); //Solucion EC31: blink color veces y freq
    }
    else
    {
        //Solucion EC31: Si el parámetro no es correcto, muestro la ayuda
        if ((data.veces<=0)||(data.frecuencia<=0))
        {
             xSerialPrintf(0," blink [veces] [frecuencia]\r\n");
        }
        else
        {
            //Solucion EC31: Si el primer argumento es red, mando un 0. Si es green mando un 1
            if (0==strncmp( argv[1], "red",3))
            { data.color=0;}
            else if (0==strncmp( argv[1], "green",5))
            {data.color=1;}

            //Solucion EC31: Almaceno los datos del argumento 2 y 3 (veces y frecuencia), en la estructura
            data.veces=strtoul(argv[2],NULL,10); //Solucion EC31: cantidad de parpadeos (numero entero)
            data.frecuencia=strtof(argv[3],NULL); //Solucion EC31: frecuencia (en punto flotante)

            //Solucion EC31: Envio la estructura a la cola1
            xQueueSend(cola1,&data,portMAX_DELAY);
        }
    }

    return 0;
}

//Solucion EC31: Creo la funcion para encender los leds con la lectura de la cola
static void LEDsOnOff( void *pvParameters )
{
    //Solucion EC31: Defino un tipo de datos en forma estructura llamada Data
    typedef struct {
        int color;
        uint16_t veces;
        float frecuencia;
    } Data;

    uint16_t i; //Solucion EC31: Variable i para recorrer el dato de veces
    //
    Data data_recibido; //Solucion EC31: Creo la variable data_recibido de tipo Data

    for(;;){

        //Solucion EC31: Recibo los datos de la cola
        xQueueReceive(cola1,&data_recibido,portMAX_DELAY);

        //Solucion EC31: Si el color a encender es el rojo...
        if (data_recibido.color == 0)
        {
            //Solucion EC31: Enciendo y apago las veces determinadas y a la frecuencia determinada
            for (i = 0; i < data_recibido.veces; i++) {

                //Solucion EC32: Añado este if para que en el caso de que se active el semaforo, paramos la tarea y borramos la cola
                if (xSemaphoreTake(xStopLEDsOnOffSemaphore, 0) == pdTRUE) {
                    GPIO_setOutputLowOnPin(LED1_PORT, LED1_PIN); //Solucion EC32: Apagamos los LEDs
                    GPIO_setOutputLowOnPin(LED2_PORT, LED2_PIN);

                    xQueueReset(cola1); //Solucion EC32: Reseteamos la cola
                    vTaskSuspend(NULL); //Solucion EC32: Suspende la tarea actual
                }

                //Solucion EC32: Si el semaforo no está activo, hace los parpadeos y freq que tiene que hacer
                GPIO_toggleOutputOnPin(LED1_PORT,LED1_PIN);
                vTaskDelay((1/data_recibido.frecuencia)*configTICK_RATE_HZ);
                GPIO_toggleOutputOnPin(LED1_PORT,LED1_PIN);
                vTaskDelay((1/data_recibido.frecuencia)*configTICK_RATE_HZ);

                //Solucion EC33: Este semaforo cancela la tarea en curso
                if (xSemaphoreTake(xCancelLEDsOnOffSemaphore, 0) == pdTRUE) {
                    i=data_recibido.veces;
                }
            }
        }

        //Solucion EC31: Si el color a encender es el verde...
        else if (data_recibido.color == 1) //Verde
        {
            //Solucion EC31: Enciendo y apago las veces determinadas y a la frecuencia determinada
            for (i = 0; i < data_recibido.veces; i++) {

                //Solucion EC32: Añado este if para que en el caso de que se active el semaforo, paramos la tarea y borramos la cola
                if (xSemaphoreTake(xStopLEDsOnOffSemaphore, 0) == pdTRUE) {
                    GPIO_setOutputLowOnPin(LED1_PORT, LED1_PIN); //Solucion EC32: Apagamos los LEDs
                    GPIO_setOutputLowOnPin(LED2_PORT, LED2_PIN);

                    xQueueReset(cola1); //Solucion EC32: Reseteamos la cola
                    vTaskSuspend(NULL); //Solucion EC32: Suspende la tarea actual
                }

                //Solucion EC32: Si el semaforo no está activo, hace los parpadeos y freq que tiene que hacer
                GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN);
                vTaskDelay((1/data_recibido.frecuencia)*configTICK_RATE_HZ); // Comentar esta y descomentar la de abajo para comprobar la diferencia de gasto de CPU
                GPIO_toggleOutputOnPin(LED2_PORT,LED2_PIN);
                vTaskDelay((1/data_recibido.frecuencia)*configTICK_RATE_HZ);

                //Solucion EC33: Este semaforo cancela la tarea en curso
                if (xSemaphoreTake(xCancelLEDsOnOffSemaphore, 0) == pdTRUE) {
                    i=data_recibido.veces;
                }
            }
        }
    }
}

//Solucion EC33: Creo la funcion para cancelar la la tarea en curso
static void Cmd_cancel( void *pvParameters ){

    //Solucion EC33: Dar el semáforo para cancelar la tarea
    xSemaphoreGive(xCancelLEDsOnOffSemaphore);

}

#if configGENERATE_RUN_TIME_STATS
// ==============================================================================
// Implementa el comando "Stats"
// ==============================================================================
static Cmd_stats(int argc, char *argv[])
{
	char*	pBuffer;
	unsigned char*	pStack;
	portBASE_TYPE	x;

	pBuffer = pvPortMalloc(1024);
	if (pBuffer)
	{
		vTaskGetRunTimeStats(pBuffer);
		xSerialPrintf(0,"TaskName\tCycles\t\tPercent\r\n");
		xSerialPrintf(0,"===============================================\r\n");
		xSerialPrintf(0,"%s", pBuffer);
		vPortFree(pBuffer);
	}
	return 0;
}
#endif


// ==============================================================================
// Tabla con los comandos y su descripción. Si quiero añadir alguno, debo hacerlo aquí
// ==============================================================================
const tCmdLineEntry g_psCmdTable[] =
{
    { "help",     Cmd_help,      "     : Lista de comandos" },
    { "?",        Cmd_help,      "        : lo mismo que help" },
    { "led",  	  Cmd_led,       "  : Apaga y enciende el led rojo o el led verde" },
    { "blink",      Cmd_blink,       "  : Apaga y enciende el led rojo o el led verde las veces y frecuencia especificada" }, //Solucion EC31: Comentario modificado
    { "free",     Cmd_free,      "     : Muestra la memoria libre" },
    { "cancel",    Cmd_cancel,     "     : Detiene el parpadeo en curso previamente puesto en marcha" },
#if ( configUSE_TRACE_FACILITY == 1 )
	{ "tasks",    Cmd_tasks,     "    : Muestra informacion de traza de las tareas" },
#endif
#if (configGENERATE_RUN_TIME_STATS)
	{ "stats",    Cmd_stats,      "     : Muestra las estadisticas " },
#endif
    { 0, 0, 0 }
};

// ==============================================================================
// Tarea UARTTask.  Espera la llegada de comandos por el puerto serie y los ejecuta al recibirlos...
// ==============================================================================

static void vUARTTask( void *pvParameters )
{
    char    cCmdBuf[64];
    int     nStatus;
	

	xSerialPrintf(0,"\r\n\r\n FreeRTOS %s \r\n",
		tskKERNEL_VERSION_NUMBER);
	xSerialPrintf(0,"\r\n Teclee ? para ver la ayuda \r\n");
	xSerialPrintf(0,"> ");

	/* Loop forever */
    while (1)
    {
		/* Wait for a signal indicating we have an RX line to process */
 		if (xSerialGetStringWithHistory( 0, cCmdBuf, sizeof(cCmdBuf))>0)
 		//if (xSerialGetString( 0, cCmdBuf, sizeof(cCmdBuf))>0)
 		{
	        //
	        // Pass the line from the user to the command processor.  It will be
	        // parsed and valid commands executed.
	        //
	        nStatus = CmdLineProcess(cCmdBuf);
	
	        //
	        // Handle the case of bad command.
	        //
	        if(nStatus == CMDLINE_BAD_CMD)
	        {
	            xSerialPrintf(0,"Comando erroneo!\r\n");	//No pongo acentos adrede
	        }
	
	        //
	        // Handle the case of too many arguments.
	        //
	        else if(nStatus == CMDLINE_TOO_MANY_ARGS)
	        {
	        	xSerialPrintf(0,"El interprete de comandos no admite tantos parametros\r\n");	//El máximo, CMDLINE_MAX_ARGS, está definido en cmdline.c
	        }
	
	        //
	        // Otherwise the command was executed.  Print the error code if one was
	        // returned.
	        //
	        else if(nStatus != 0)
	        {
	        	xSerialPrintf(0,"El comando devolvio el error %d\r\n",nStatus);
	        }
	
	        xSerialPrintf(0,"> ");
	    }
    }
}

void initCommands (void)
{
    cola1 = xQueueCreate(10,sizeof(Data)); //Solucion EC31: Creando la cola
    if (cola1==NULL) //Solucion EC31: Me aseguro de que esta bien creada
    {
        while(1);
    }

    //Solucion EC33: Creo una semaforo que se active al teclear "cancel"
    xCancelLEDsOnOffSemaphore = xSemaphoreCreateBinary();
    if( xCancelLEDsOnOffSemaphore == NULL )
        {
                 while(1);
        }

    //Solucion EC31: Creo la tarea que va a gestionar el los Leds con los datos enviados por comandos
    if((xTaskCreate(LEDsOnOff, "LEDsOnOff", configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY + 1, NULL) != pdPASS))
        {
            while(1);
        }


    /* luego creamos las tareas */
    if((xTaskCreate(vUARTTask, "Cmd", 256,NULL,tskIDLE_PRIORITY + 1, NULL) != pdTRUE))
    {
        while(1);
    }
}
