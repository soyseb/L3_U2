 // FileName:        HVAC_Thread.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Definición de las funciones del thread de HVAC_Thread().
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "hvac.h"                           // Incluye definición del sistema.


/*********************************THREAD******************************************
 * Function: HVAC_Thread
 * Preconditions: None.
 * Overview: Realiza la lectura de la temperatura y controla salidas actualizando
 *           a su vez entradas. Imprime estados. También contiene el heartbeat.
 * Input:  Apuntador vacío que puede apuntar cualquier tipo de dato.
 * Output: None.
 *
 ********************************************************************************/

void *HVAC_Thread(void *arg0)
{
    SystemInit();
    HVAC_InicialiceIO();
    HVAC_InicialiceADC();
    HVAC_InicialiceUART();

    while(1)
    {
        HVAC_ActualizarEntradas();
        HVAC_ActualizarSalidas();
        HVAC_PrintState();
        HVAC_Heartbeat();
    }
}

