 // FileName:        HVAC.h
 // Dependencies:    None.
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librerías, define ciertas macros y significados así como llevar un control de versiones.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.

// Archivos de cabecera importantes.
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Archivos de cabecera POSIX.
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

// Archivos de cabecera para RTOS.
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

// Board Support Package.
#include "Drivers/BSP.h"

/* Enumeradores para la descripción del sistema. */

enum SYSTEM
{
    On,
    Off,
    Up,
    Down,
};

/*Se establece una estructura de las entradas que se van a usar*/
struct EstadoEntradas
{
    uint8_t  SystemState;
    uint8_t  Damper1State;
    uint8_t  Damper2State;
    uint8_t  SequenceState;
}   EstadoEntradas;

/* Definiciones básicas de puertos y bits que se van a usar.*/
#define ENTRADA 1
#define SALIDA 0

#define SEQUENCE_LEDS_PORT    3
#define SYSTEM_ON_PORT        3
#define DAMPERS_PORT          1
#define SEQUENCE_LEDS_PORTT   P3
#define SYSTEM_ON_PORTT       P3
#define DAMPERS_PORTT         P1
#define SYSTEM_ON_BIT         B5
#define SEQUENCE_LEDS_BIT     B7
#define DAMPER1_BIT           B1
#define DAMPER2_BIT           B4

/* Canales del ADC que se van a utilizar*/
#define LUM1    CH8
#define LUM2    CH9
#define LUM3    CH10

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383             // (2 ^14 bits) es la resolución default.
#define MAIN_UART (uint32_t)(EUSCI_A0)
#define DELAY 20000
#define ITERATIONS_TO_PRINT 49

// Definición para el RTOS.
#define THREADSTACKSIZE1 1500

/* Funciones que se habilitan con las interrupciones. */
extern void INT_DAMPERS (void);
extern void INT_SEQUENCE_LEDS (void);

// Funciones de inicialización. //
extern void HVAC_InicialiceIO   (void);
extern void HVAC_InicialiceADC  (void);
extern void HVAC_InicialiceUART (void);
extern void System_InicialiceTIMER (void); // ESTO LO AÑADI PARA UTILIZAR EL "TIMER32"

// Funciones principales.//
extern void HVAC_ActualizarEntradas(void);
extern void HVAC_PrintState(void);
extern void fun_Damper1(void); //Funciones que se usarán cuando haya interrupciones.
extern void fun_Damper2(void); //

#endif
