/*! @mainpage Ejercicio 2 Proyecto 2
 *
 * @section genDesc General Description
 * Este proyecto implementa el sensor de distancia HC-SR04 para medir la distancia
 * y mostrarlo en un display LCD, asi como tambien una secuencia de LEDS que se encienden de acuerdo
 * y teclas para on/off y Hold. en este ejercicio se implementa ademas interrupciones y timers
 *
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * |   TRIG	 	    | 	GPIO_2		|
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/09/2024 | Document creation		                         |
 *
 * @author Alvaro Ojeda
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief Delay para medir la distancia por el HC-SR04
 */
#define DELAY_MEDIR_uS 200000

/**
 * @brief Delay para mostrar la distancia en el LCD
 */
#define DELAY_LCD_uS 1000000

/**
 * @brief Variable global que almacena si el sensor se encuentra encendido/apagado 
 */
bool encendido = true;

/**
 * @brief Variable global que almacena si Hold esta activo
 */
bool hold = false;

/**
 * @brief Variable global que almacena la distancia medida por el HC-SR04
 */
uint16_t distancia = 0;

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

void FuncTimerMedir(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

void FuncTimerMostrar(void *param)
{
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_2 */
}

/**
 * @brief Controla los LEDs segun la distancia medida por el HC-SR04
 */
void ControlarLEDs(void)
{
	// Tiene que fijarse que leds prender y apagar
	if (distancia < 10)
	{
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia < 20)
	{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else if (distancia < 30)
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else
	{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}

/**
 * @brief Tarea encargada de medir la distancia con el sensor HC-SR04
 *
 * Esta tarea se encarga de medir la distancia con el sensor HC-SR04 y
 * actualizar la variable global "distancia".
 */
static void MedirDistancia(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (encendido)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		else
		{
			LcdItsE0803Off();
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		
	}
}

/**
 * @brief Interrupción para el switch 1
 *
 * Esta función es invocada cuando se produce una interrupción en el switch 1.
 * La función cambia el valor de la variable global "encendido"
 */
void InterrupcionTecla1(void)
{
	encendido = !encendido;
}

/**
 * @brief Interrupción para el switch 2
 *
 * Esta función es invocada cuando se produce una interrupción en el switch 2.
 * La función cambia el valor de la variable global "hold"
 */
void InterrupcionTecla2(void)
{
	hold = !hold;
}

/**
 * @brief Tarea encargada de mostrar en el display LCD la distancia medida
 * por el sensor HC-SR04. La tarea tambien se encarga de controlar los LEDs
 * según la distancia medida.

 */
static void MostrarDisplayLCD(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (encendido)
		{
			if (hold)
			{
				ControlarLEDs();
			}
			else
			{
				LcdItsE0803Write(distancia);
				ControlarLEDs();
			}
		}
		
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	// Inicializo el LCD, LEDs,switches y el sensor
	LcdItsE0803Init();
	SwitchesInit();
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);

	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = DELAY_MEDIR_uS,
		.func_p = FuncTimerMedir,
		.param_p = NULL};
	TimerInit(&timer_medir);

	timer_config_t timer_mostrar = {
		.timer = TIMER_B,
		.period = DELAY_LCD_uS,
		.func_p = FuncTimerMostrar,
		.param_p = NULL};
	TimerInit(&timer_mostrar);

	SwitchActivInt(SWITCH_1, InterrupcionTecla1, NULL);
	SwitchActivInt(SWITCH_2, InterrupcionTecla2, NULL);

	xTaskCreate(MostrarDisplayLCD, "MostrarDisplayLCD", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(MedirDistancia, "MedirDistancia", 512, NULL, 5, &medir_task_handle);
}
/*==================[end of file]============================================*/
