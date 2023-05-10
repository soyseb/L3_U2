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

/*****  SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DELAY CON EL TIMER ******** */
bool retraso = false;
void Timer32_INT1 (void); // Función de interrupción.
void Delay_ms (uint32_t time); // Función de delay.
/***********************************************************************************************/

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
    System_InicialiceTIMER();            // SE AÑADIO LA FUNCION PARA INICIALIZAR EL TIMER
    HVAC_InicialiceIO();
    HVAC_InicialiceADC();
    HVAC_InicialiceUART();

    EstadoEntradas.Damper1State = Off;  /*Se inicializan los estados en Off.*/
    EstadoEntradas.Damper2State = Off;
    EstadoEntradas.SequenceState = Off;
    EstadoEntradas.SystemState = Off;
    GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  0); /* Nos aseguramos que estén apagados los led*/
    GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
    GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
    GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);

    while(GPIO_getInputPinValue(SYSTEM_ON_PORT, BIT(SYSTEM_ON_BIT)) == 1);  /*Se espera para arrancar el sistema con el boton*/

    EstadoEntradas.SystemState = On; /* Al presionar el botón, el systemstate se pone en on.*/
    GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  1); /*Se enciende el led rojo.*/

    while(1) /*Se cicla en el while mandando llamar a estas dos entradas*/
    {
        HVAC_ActualizarEntradas();
        HVAC_PrintState();
    }
}



/* *********  FUNCIONES PARA REALIZAR EL DELAY CON EL TIMER ********* */
void Delay_ms(uint32_t time)
{
    T32_EnableTimer1(); // Habilita timer.
    T32_EnableInterrupt1(); // Habilita interrupción.
    // Carga de valor en milisegundos.
    T32_SetLoadValue1(time*(__SYSTEM_CLOCK/1000));
    retraso = true;
    while(retraso); // While enclavado.
}
void Timer32_INT1(void)
{
    T32_ClearInterruptFlag1(); // Al llegar a la interrupción
    retraso = false; // desenclava el while.
}

