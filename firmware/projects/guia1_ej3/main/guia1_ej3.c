/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 0
#define TOGGLE 2

/*==================[internal data definition]===============================*/

struct leds
{
	uint8_t mode;	  // ON, OFF, TOGGLE
	uint8_t n_led;	  // indica el número de led a controlar
	uint8_t n_ciclos; // indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo; // ndica el tiempo de cada ciclo
} my_leds;

/*==================[internal functions declaration]=========================*/

void FuncionLed(struct leds *ptrLed)
{
	switch (ptrLed->mode)
	{
	case ON:
		switch (ptrLed->n_led)
		{
		case 1:
			LedOn(LED_1);
			break;
		case 2:
			LedOn(LED_2);
			break;
		case 3:
			LedOn(LED_3);
			break;
		}

		break;
	case OFF:
		switch (ptrLed->n_led)
		{
		case 1:
			LedOff(LED_1);
			break;
		case 2:
			LedOff(LED_2);
			break;
		case 3:
			LedOff(LED_3);
			break;
		}
		break;
	case TOGGLE:
		for (int i = 0; i < (ptrLed->n_ciclos); i++)
		{
			if (ptrLed->n_led == 1)
				LedToggle(LED_1);

			if (ptrLed->n_led == 2)
				LedToggle(LED_2);
			else
			{
				if (ptrLed->n_led == 3)
					LedToggle(LED_3);
			}

			for (int j = 0; j < ptrLed->periodo / CONFIG_BLINK_PERIOD; j++)
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		}

		break;
	}
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	LedsInit();
	my_leds.mode=TOGGLE;
	my_leds.n_led=1;
	my_leds.n_ciclos=10;
	my_leds.periodo=500;
	FuncionLed(&my_leds);
	
}

/*==================[end of file]============================================*/