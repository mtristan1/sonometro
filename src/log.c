/*
 * log.c
 *
 *  Created on: 26/11/2012
 *      Author: Martín
 */

#include "log.h"

/**
 * @brief	Calcula x a la décima potencia (x^10)
 * @param x	Número de origen
 */
float potencia10(float x)
{
	int i;
	for(i=0; i<9; i++)
		x *= x;
	return x;
}

/**
 * @brief	Calcula el logaritmo en 'base_log' del parámetro recibido como argumento.
 * 			La cantidad de decimales está limitada a 'total_decimales'
 * 			Nota: No funciona bien para num < 1
 * @param num	Número del que se quiere obtener el logaritmo
 */
float log(float num)
{
	int entero = 0, i, digito;
	float desp = 1, resultado;

	while(num >= base_log)	// Calcula primero la parte entera del logatirmo
	{
		entero += 1;
		num /= base_log;
	}

	resultado = entero;	// La parte entera ya está resuelta

	desp = 1;	// 'desp' es una variable auxiliar para armar el número final con los dígitos descimales por separado

	for(i=0; i<total_decimales; i++)	// Ahora calcula cada dígito decimal por separado
	{
		num = potencia10(num);	// Eleva num a la 10º potencia
		desp = desp/10;			// Se mueve en 1 decimal hacia la derecha
		digito = 0;
		while(num >= base_log)
		{
			digito = digito+1;
			num /= base_log;
		}
		resultado += digito * desp;		// Obtiene el digito en el lugar decimal correspondiente
	}

	return resultado;
}
