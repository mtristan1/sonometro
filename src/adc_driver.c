/*
 * adc_driver.c
 *
 *  Created on: 27/10/2012
 *      Author: Martín
 */

#include "LPC17xx.h"
#include "adc_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


// Variables globales:

Acumuladores datos;
int conversiones_totales;	///< Indica el nº total de conversiones que el ADC realiza en 1 segundo
int lecturas;				///< Lleva la cuenta de las conversiones realizadas (sobre los 4 canales)
xQueueHandle qLecturaADC;	///< Cola de salida para los datos leídos por el ADC. (Debe declararse como extern en el archivo main)


//=============================================================================================================

/**
 * @brief 	Inicializa el módulo ADC. Habilita los 4 canales de conversión utilizados.
 * 			El módo ráfaga inicia deshabilitado (requiere habilitación posterior mediante 'ADC_IniciaRafaga').
 * 			Inicializa todas las variables y crea la cola de datos.
 *
 * @param 	ADC_Clk Frecuencia de muestreo para los 4 canales de conversión (es decir que cada canal muestrea a ADC_Clk/4)
 */
void ADC_Init(int ADC_Clk)
{

	int pclkdiv, i, dummy;

	LPC_SC->PCONP |= (1 << 12);	// Habilita la alimentación del ADC

	/*
	 * El valor de reset de los bits PCLK_ADC (en el resistro PCLKSEL0) es 00,
	 * por lo que el periférico del ADC funciona por omisión a 1/4 de la frecuencia
	 * del sistema (CCLK):
	 */

	pclkdiv = (SystemCoreClock >> 2) / ADC_Clk;	// SystemCoreClock >> 2 = SystemCoreClock/4
	pclkdiv--;

	// Habilita la función de ADC (AD0[0:3]) en los pines correspondientes (P0[23:26])

	LPC_PINCON->PINSEL1 &= ~(1<<15);	//
	LPC_PINCON->PINSEL1 |=  (1<<14);	// P0.23 -> AD0.0 (Función 01)

	LPC_PINCON->PINSEL1 &= ~(1<<17);	//
	LPC_PINCON->PINSEL1 |=  (1<<16);	// P0.24 -> AD0.1 (Función 01)

	LPC_PINCON->PINSEL1 &= ~(1<<19);	//
	LPC_PINCON->PINSEL1 |=  (1<<18);	// P0.25 -> AD0.2 (Función 01)

	LPC_PINCON->PINSEL1 &= ~(1<<21);	//
	LPC_PINCON->PINSEL1 |=  (1<<20);	// P0.26 -> AD0.3 (Función 01)

	// Deshabilita las resistencias de pull-up/down en los pines P0[23:26]

	LPC_PINCON->PINMODE1 |= (1<<15);	//
	LPC_PINCON->PINMODE1 &= ~(1<<14);	// P0.23 (Función 10)

	LPC_PINCON->PINMODE1 |= (1<<17);	//
	LPC_PINCON->PINMODE1 &= ~(1<<16);	// P0.24 (Función 10)

	LPC_PINCON->PINMODE1 |= (1<<19);	//
	LPC_PINCON->PINMODE1 &= ~(1<<18);	// P0.25 (Función 10)

	LPC_PINCON->PINMODE1 |= (1<<21);	//
	LPC_PINCON->PINMODE1 &= ~(1<<20);	// P0.26 (Función 10)


	LPC_ADC->ADCR = ( 1 << 0 ) | ( 1 << 1 ) | ( 1 << 2) | ( 1 << 3 ); // Selecciona el canal de conversiÃ³n AD0.5
	LPC_ADC->ADCR |= ( pclkdiv << 8 ); /* CLKDIV = pclkdiv */
	LPC_ADC->ADCR |=( 0 << 16 );  		/* BURST = 0, el conversor no inicia en modo rÃ¡faga */
	LPC_ADC->ADCR |=( 1 << 21 );  		/* PDN = 1, normal operation */


	dummy = LPC_ADC->ADGDR;	//
	dummy = LPC_ADC->ADDR0;	//
	dummy = LPC_ADC->ADDR1;	//
	dummy = LPC_ADC->ADDR2;	// Limpia todos los flags de los registros de conversión
	dummy = LPC_ADC->ADDR3;	//
	dummy = LPC_ADC->ADDR4;	//
	dummy = LPC_ADC->ADDR5;	//
	dummy = LPC_ADC->ADDR6;	//
	dummy = LPC_ADC->ADDR7;	//


	LPC_ADC->ADINTEN = 0x08;		// Solo el canal AD3 genera interrupciones
	LPC_ADC->ADINTEN &= (~(1<<8));

	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 8);	// Configura la prioridad de la int. por ADC en 8 (necesario para usar funciones del RTOS en las int.)

	qLecturaADC = xQueueCreate(1, sizeof(Acumuladores));

	for(i=0; i<3; i++)	// Inicializa acumuladores
	{
		datos.v_lin[i] = 0;
		datos.v_sq[i] = 0;
	}

	conversiones_totales = ADC_Clk/4;	// El nº total de muestras en 1 segundo es igual a la frecuencia de muestreo
	lecturas = 0;		// Inicia la cuenta de las lecturas realizadas
}


/**
 * @brief	Habilita el módo ráfaga del ADC, generando conversiones de manera automática periodicamente.
 */
void ADC_IniciaRafaga(void)
{
	LPC_ADC->ADCR |= ( 1 << 16 );	// BURST: On
}


/**
 * @brief	Deshabilita el módo ráfaga del ADC.
 */
void ADC_DetieneRafaga(void)
{
	LPC_ADC->ADCR &= ~( 1 << 16 );	// BURST: Off
}


/**
 * @brief	Selecciona mediante el parámetro 'canal' el canal de conversión del ADC0;
 * 			canal varía entre 0 y 3.
 *
 * TODO: Creo que esta función está al pedo
 */
void ADC_SetCanal(int canal)
{
	int aux;

	aux = LPC_ADC->ADCR;
	aux &= 0xFFFFFF00;		// Enmascara los primeros 8 bits ADCR

	switch(canal)
	{
		case 0: aux |= (1<<0); break;
		case 1: aux |= (1<<1); break;
		case 2: aux |= (1<<2); break;
		case 3: aux |= (1<<3); break;
		default: break;
	}

	LPC_ADC->ADCR = aux;	// Selecciona el canal correspondiente
}


/**
 * @brief	Lee periódicamente la conversión del ADC, calcula el cuadrado de la muestra y acumula los resultados
 *			en la estructura 'datos'. El resultado de cada conversión se trunca en 8 bits, para evitar problemas
 * 			de overflow.  Luego de transcurrido 1 segundo, envía el resultado de todos los acumuladores a la cola
 * 			qLecturaADC, y reinicia todas las variables.
 */
void ADC_IRQHandler(void)
{
	unsigned int aux, canal;
	int i;
	portBASE_TYPE tarea_desbloqueada;

	aux = LPC_ADC->ADGDR;		// Lee el resultado del 'AD Global Data Register'
	canal = (aux >> 24) & 0x7;	// Obtiene el nº de canal en el cual se realizó la converión

	aux = LPC_ADC->ADDR0;		// Calcula para el canal 0
	aux = (aux >> 11) & 0xFF;	// Enmascara, recorta los 4 bits menos significativos de la conversión y extrae el resultado en aux
	datos.v_lin[0] += aux;		// Incrementa el acumulador de promedio lineal
	datos.v_sq[0] += aux*aux;	// Incrementa el acumulador de promedio cuadrático

	aux = LPC_ADC->ADDR1;		// Calcula para el canal 1
	aux = (aux >> 11) & 0xFF;
	datos.v_lin[1] += aux;
	datos.v_sq[1] += aux*aux;

	aux = LPC_ADC->ADDR2;		// Calcula para el canal 2
	aux = (aux >> 11) & 0xFF;
	datos.v_lin[2] += aux;
	datos.v_sq[2] += aux*aux;

	aux = LPC_ADC->ADDR3;		// Calcula para el canal 3
	aux = (aux >> 11) & 0xFF;
	datos.v_lin[3] += aux;
	datos.v_sq[3] += aux*aux;

	lecturas++;
	if(lecturas == conversiones_totales)
	{
		lecturas = 0;
		xQueueSendFromISR(qLecturaADC, &datos, &tarea_desbloqueada);	// Manda la estructura de acumuladores a la cola

		for(i=0; i<3; i++)	// Inicializa acumuladores
		{
			datos.v_lin[i] = 0;
			datos.v_sq[i] = 0;
		}
	}
	portEND_SWITCHING_ISR(tarea_desbloqueada);	// Fuerza un cambio de contexto al finalizar la interrupción
}
