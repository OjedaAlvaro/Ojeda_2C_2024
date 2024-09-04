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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
#define DELAY_MEDIR 200
#define DELAY_LCD 1000
bool encendido = true;
bool hold = false;
uint16_t distancia = 0;
/*==================[internal functions declaration]=========================*/

void ControlarLEDs(void){
	//Tiene que fijarse que leds prender y apagar
	if (distancia<10){
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}else if(distancia<20){
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}else if(distancia<30){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}else{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}

static void MedirDistancia(void *pvParameter)
{
	while (true)
	{
		if (encendido)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}else {
			LcdItsE0803Off();
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
	}
}

static void LeerTeclas(void *pvParameter)
{
	uint8_t teclas;
	//SwitchesInit();
	while (true)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			encendido = !encendido;
			break;
		case SWITCH_2:
			hold = !hold;
			break;
		}
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
	}
}

static void MostrarDisplayLCD(void *pvParameter)
{
	while (true)
	{
		if(encendido){
			if(hold){
				ControlarLEDs();
			}else{
				LcdItsE0803Write(distancia);
				ControlarLEDs();
			}
		}
		vTaskDelay(DELAY_LCD / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{	
	//Inicializo el LCD, LEDs,switches y el sensor
	LcdItsE0803Init();
	SwitchesInit();
	LedsInit();
	HcSr04Init(GPIO_3,GPIO_2);
	//Creo las tareas
	xTaskCreate(MostrarDisplayLCD, "MostrarDisplayLCD", 512, NULL, 5, NULL);
	xTaskCreate(LeerTeclas, "LeerTeclas", 512, NULL, 5, NULL);
	xTaskCreate(MedirDistancia, "MedirDistancia", 512, NULL, 5, NULL);
	
	
}
/*==================[end of file]============================================*/
