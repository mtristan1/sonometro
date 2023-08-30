/*
 * adc_driver.h
 *
 *  Created on: 27/10/2012
 *      Author: Martín
 */

#ifndef ADC_DRIVER_H_
#define ADC_DRIVER_H_
#endif /* ADC_DRIVER_H_ */

// Definiciones
#define AD_x1		3	//
#define AD_x10		2	// Nº de cada canal de conversión
#define AD_x100		1	// TODO: Verificar que el nº de cada canal corresponda con el conexionado de la placa
#define AD_x1000	0	//

// Prototipos:
void ADC_Init(int);
void ADC_IniciaRafaga(void);
void ADC_DetieneRafaga(void);
void ADC_SetCanal(int);
void ADC_IRQHandler(void);

/**
 * @brief	Estructura que contiene los 2 acumuladores por canal, uno para la sumatoria
 * 			cuadrática y otro para la sumatoria lineal, con los que luego se calcula
 * 			el valor eficaz de la señal.
 */
typedef struct
{
	unsigned int v_sq[4];	// 4 enteros para el acumulador cuadrático ([0]: ADCx1 ... [3]:ADCx1000)
	unsigned int v_lin[4];	// 4 enteros para el acumulador lineal
} Acumuladores;

