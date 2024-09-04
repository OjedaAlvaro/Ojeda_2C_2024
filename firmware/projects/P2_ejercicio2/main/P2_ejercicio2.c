/*! @mainpage Ejercicio 1 Proyecto 2
 *
 * @section Este proyecto implementa el sensor de distancia HC-SR04 para medir la distancia
 * y mostrarlo en un display LCD, asi como tambien una secuencia de LEDS que se encienden de acuerdo
 * y teclas para on/off y Hold
 *
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
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
#define DELAY_MEDIR_uS 200000
#define DELAY_LCD_uS 1000000
bool encendido = true;
bool hold = false;
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

void InterrupcionTecla1(void)
{
	encendido = !encendido;
}

void InterrupcionTecla2(void)
{
	hold = !hold;
}

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
