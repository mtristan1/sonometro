/**
 * Name        : main.c
 * Version     :
 * Description : main definition for FreeRTOS application
 */

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "adc_driver.h"
#include "log.h"

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE

// Definiciones:
#define adc_fs	160000	// 160kHz (compartida por los 4 canales)
#define rango_max	3865470566	// 90% de 2^32 (estimativo)

void __error__(char *pcFilename, unsigned long ulLine) {
}

// Variables globales:
float cte_calibracion = 1;

// Colas:
xQueueHandle qNRE;
extern xQueueHandle qLecturaADC;
extern int conversiones_totales;

//=============================================================================================================

static void setupHardware(void)
{
	// TODO: Put hardware configuration and initialisation in here
	
	ADC_Init(adc_fs);
}


/**
 * @brief	Calcula el NRE (Nivel de Ruido Equivalente) a partir de los datos suministrados por la int. del ADC
 * 			El resutado es enviado a la cola qNRE, que deberá ser leída por la tarea del display
 */
void tCalculaNRE(void *pvParameters)
{
	Acumuladores datos;
	unsigned int resultado;
	int i, canal;
	float resultado_log;

	ADC_IniciaRafaga();		// Inicia las conversiones del ADC

	while(1)
	{
		xQueueReceive(qLecturaADC, &datos, portMAX_DELAY);	// Lee los datos de la cola

		if(datos.v_sq[3] < rango_max)	// Comprueba ADCx1000
			canal = 3;
		else
		{
			if(datos.v_sq[2] < rango_max)	// Comprueba ADCx100
				canal = 2;
			else
			{
				if(datos.v_sq[1] < rango_max)	// Comprueba ADCx10
					canal = 1;
				else
					canal = 0;	// Como última opción, toma directamente ADCx1
			}
		}

		resultado = datos.v_sq[canal] - datos.v_lin[canal]*datos.v_lin[canal];	// Valor cuadártico medio - Valor medio cuadrado

		resultado_log = log((float)resultado);		// Calcula el logaritmo del resultado para pasar la medida a dB
		resultado_log -= 10*canal;			// Resta 10dB por cada década de amplificación anterior
		resultado_log *= cte_calibracion;	// Multiplica por constante de calibración

		xQueueSend(qNRE, &resultado_log, portMAX_DELAY);	// Manda el resultado a la cola del display
	}
}


/**
 * Program entry point 
 */
int main(void)
{
	setupHardware();

	xTaskCreate( tCalculaNRE, ( signed portCHAR * ) "NRE", USERTASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
	qNRE = xQueueCreate(1, sizeof(float));


	/* 
	 * Start the scheduler. 
	 */
	vTaskStartScheduler();

	/* 
	 * Will only get here if there was insufficient memory to create the idle task. 
	 */
	return 1;
}
