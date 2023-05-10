 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

/* Variables sobre las cuales se maneja el sistema. */
float lum_1=0;
float lum_2=0;
float lum_3=0;

char state[MAX_MSG_SIZE];      // Cadena a imprimir .

bool event = FALSE;            // Banderas para evento I/O que fuerza impresión inmediata.
bool eventSequence = FALSE;    /* Bandera para la secuencia*/

/* **** SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DELAY CON EL TIMER ******** */
extern void Timer32_INT1 (void); // Función de interrupción.
extern void Delay_ms (uint32_t time); // Función de delay.
uint32_t tiempo = 2000;

/*FUNCTION******************************************************************************
*
* Function Name    : System_InicialiceTIMER
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar TIMER32
*
*END***********************************************************************************/
void System_InicialiceTIMER (void)
{
    T32_Init1();
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);
    Int_enableInterrupt(INT_T32_INT1);
}

/*  */

void INT_DAMPERS(void)
{
    GPIO_clear_interrupt_flag(P1,B1); // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P1,B4); // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(DAMPERS_PORT,BIT(DAMPER1_BIT)))        // Si se trata de la persiana 1.
        fun_Damper1();
    else if(!GPIO_getInputPinValue(DAMPERS_PORT,BIT(DAMPER2_BIT))) // Si se trata de la persiana 2.
        fun_Damper2();

    return;
}


void INT_SEQUENCE_LEDS (void){
    GPIO_clear_interrupt_flag(P3,B7); /*Limpia la bandera de la interrupción.*/

    eventSequence = TRUE; /*Pone la bandera en true*/

    if(EstadoEntradas.SequenceState == Off) /*Toggle button para cambiar el estado de la secuencia*/
        EstadoEntradas.SequenceState = On;
    else if (EstadoEntradas.SequenceState == On)
        EstadoEntradas.SequenceState = Off;
}
/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar las entradas y salidas GPIO.
*
*END***********************************************************************************/
void HVAC_InicialiceIO(void)
{
    // Para entradas y salidas ya definidas en la tarjeta.
    GPIO_init_board();

    /* Modo de interrupción de los botones principales.*/ //flanco de bajada al presionar el botón
    GPIO_interruptEdgeSelect(DAMPERS_PORT,BIT(DAMPER1_BIT),   GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(DAMPERS_PORT,BIT(DAMPER2_BIT),   GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(SEQUENCE_LEDS_PORT,BIT(SEQUENCE_LEDS_BIT), GPIO_HIGH_TO_LOW_TRANSITION);

    /* Preparativos de interrupción, limpia las banderas al inicializar*/
    GPIO_clear_interrupt_flag(DAMPERS_PORTT ,DAMPER1_BIT); //Persiana 1
    GPIO_clear_interrupt_flag(DAMPERS_PORTT ,DAMPER2_BIT); //Persiana 2
    GPIO_clear_interrupt_flag(SEQUENCE_LEDS_PORTT,SEQUENCE_LEDS_BIT); //Secuencia LEDs

    /*Habilita las interrupciones*/
    GPIO_enable_bit_interrupt(DAMPERS_PORTT ,DAMPER1_BIT); //Persiana 1
    GPIO_enable_bit_interrupt(DAMPERS_PORTT ,DAMPER2_BIT); //Persiana 1
    GPIO_enable_bit_interrupt(SEQUENCE_LEDS_PORTT,SEQUENCE_LEDS_BIT); //Secuencia LEDs

    // Se necesitan más entradas, se usarán las siguientes:
    GPIO_setBitIO(SEQUENCE_LEDS_PORT, SEQUENCE_LEDS_BIT, ENTRADA);
    GPIO_setBitIO(SYSTEM_ON_PORT, SYSTEM_ON_BIT, ENTRADA);

    //Interrupcion para cualquiera de las persianas
    Int_registerInterrupt(INT_PORT1, INT_DAMPERS); /*Enlaza interrupción con la función*/
    Int_enableInterrupt(INT_PORT1);

    //Interrupcion para la secuencia de los LEDs
    Int_registerInterrupt(INT_PORT3, INT_SEQUENCE_LEDS); /*Enlaza interrupción con la función*/
    Int_enableInterrupt(INT_PORT3);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el módulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales.
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannels);
    ADC_ConfigurePinChannel(LUM1, AN8, ADC_VCC_VSS); /*pin 4.5. se enlaza el canal a la entrada analoga de la tarjeta para realizar la lectura*/
    ADC_ConfigurePinChannel(LUM2, AN9, ADC_VCC_VSS); /*pin 4.4.*/
    ADC_ConfigurePinChannel(LUM3, AN10, ADC_VCC_VSS); /*Pin 4.3*/
    ADC_SetStartOfSequenceChannel(AN8); /*Empieza la secuencia de lectura en el análogo 8*/
    ADC_SetEndOfSequenceChannel(AN10); /*Termina la secuencia de lectura en el análogo 10*/
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
void HVAC_InicialiceUART (void)
{
    UART_init();
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    ADC_trigger(); while(ADC_is_busy());
    lum_1 = (ADC_result(LUM1) * 10) / MAX_ADC_VALUE; //Hace la lectura del ADC y escala el valor de 0 a 10 para los 3 canales*/

    ADC_trigger(); while(ADC_is_busy());
    lum_2 = (ADC_result(LUM2) * 10) / MAX_ADC_VALUE;

    ADC_trigger(); while(ADC_is_busy());
    lum_3 = (ADC_result(LUM3) * 10) / MAX_ADC_VALUE;
}


/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static char iterations = 0;
    const char* Damper_string[]={"ON","OFF","UP","DOWN"}; /*Arreglo de apuntadores tipo char para cuatro cadenas de caractéres*/

    iterations++; // Aumenta una unidad en las iteraciones
    if(iterations >= ITERATIONS_TO_PRINT || eventSequence == TRUE) /*Entra a este if si se cumplieron las 50 iteraciones o se presionó el botón de eventSequences*/
    {
        iterations = 0; /* Vuelve a poner las iteraciones en 0*/
        eventSequence = FALSE; /*Vuelve a poner la bandera en false*/

        sprintf(state,"LUM_1: %0.2f, LUM_2: %0.2f, LUM_3: %0.2f, ", /*Guarda los datos escalados en la cadena "State" */
                lum_1,
                lum_2,
                lum_3);
        UART_putsf(MAIN_UART,state); /*Imprime la cadena "State" en la terminal*/

        sprintf(state,"Persiana1: %s, Persiana2: %s, SL= %s\n\r", /*Imprime el estado de las persianas mediante el uso de la cadena Damper_string*/
                Damper_string[EstadoEntradas.Damper1State],
                Damper_string[EstadoEntradas.Damper2State],
                EstadoEntradas.SequenceState == On? "ON":"OFF");
        UART_putsf(MAIN_UART,state);
    }

    if (event == TRUE)
    {
        sprintf(state,"LUM_1: %0.2f, LUM_2: %0.2f, LUM_3: %0.2f, ",
                lum_1,
                lum_2,
                lum_3);
        UART_putsf(MAIN_UART,state);

        sprintf(state,"Persiana1: %s, Persiana2: %s, SL= %s\n\r",
                Damper_string[EstadoEntradas.Damper1State],
                Damper_string[EstadoEntradas.Damper2State],
                EstadoEntradas.SequenceState == On? "ON":"OFF");
        UART_putsf(MAIN_UART,state);

        Delay_ms(5000);
        if(EstadoEntradas.Damper1State == Up)
            EstadoEntradas.Damper1State = On;
        else if(EstadoEntradas.Damper1State == Down)
            EstadoEntradas.Damper1State = Off;

        if(EstadoEntradas.Damper2State == Up)
            EstadoEntradas.Damper2State = On;
        else if(EstadoEntradas.Damper2State == Down)
            EstadoEntradas.Damper2State = Off;

        event = FALSE;
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : fun_Damper1
* Returned Value   : None.
* Comments         :
*    Sube o baja la persiana 1
*
*END***********************************************************************************/
void fun_Damper1(void)
{
    event = TRUE; /*Activa la bandera para imprimir*/

    if(EstadoEntradas.Damper1State == Off)
        EstadoEntradas.Damper1State = Up;

    else if (EstadoEntradas.Damper1State == On)
        EstadoEntradas.Damper1State = Down;
}

/*FUNCTION******************************************************************************
*
* Function Name    : fun_Damper1
* Returned Value   : None.
* Comments         :
*    Sube o baja la persiana 2
*
*END***********************************************************************************/
void fun_Damper2(void)
{
    if(EstadoEntradas.Damper2State == Off)
        EstadoEntradas.Damper2State = Up;

    else if (EstadoEntradas.Damper2State == On)
        EstadoEntradas.Damper2State = Down;

    event = TRUE; /*Activa la bandera para imprimir*/
}
