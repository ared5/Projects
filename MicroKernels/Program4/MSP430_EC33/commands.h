//Solucion EC32: Defino la cola aqui para poder ser usada en el main.c y en commands.c

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "FreeRTOS.h"
#include "queue.h"

//Solucion EC32: Definición de la estructura de datos que vas a almacenar en la cola
typedef struct {
    int color;
    uint16_t veces;
    float frecuencia;
} Data;

//Solucion EC31: Declaración de la cola1
extern QueueHandle_t cola1;

// Función que inicializa el interprete de comandos y la cola1
void initCommands(void);

//Solucion EC32: Declaro el semaforo para utilizarlo en commands.c
extern SemaphoreHandle_t xStopLEDsOnOffSemaphore;

#endif /* COMMANDS_H_ */
