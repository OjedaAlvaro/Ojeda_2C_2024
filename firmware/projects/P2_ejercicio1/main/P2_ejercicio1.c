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
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIG	 	| 	GPIO_2		|
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
/**
 * @def DELAY_MEDIR
 * @brief Delay para medir la distancia por el HC-SR04
 */
#define DELAY_MEDIR 200

/**
 * @def DELAY_LCD
 * @brief Delay para mostrar la distancia en el LCD
 */
#define DELAY_LCD 1000

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


/*==================[internal functions declaration]=========================*/

/**
 * @brief Controla los LEDs segun la distancia medida por el HC-SR04
 */
void ControlarLEDs(void){
	// Si la distancia es menor a 10 cm, apaga todos los LEDs
	if (distancia < 10) {
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	} 
	// Si la distancia es mayor o igual a 10 cm y menor a 20 cm, prende solo el LED_1
	else if (distancia < 20) {
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	} 
	// Si la distancia es mayor o igual a 20 cm y menor a 30 cm, prende los LEDs 1 y 2
	else if (distancia < 30) {
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	} 
	// Si la distancia es mayor o igual a 30 cm, prende todos los LEDs
	else {
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
		if (encendido)
		{
			// Medir la distancia en centimetros y actualizar la variable global
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		else
		{
			// Apagar el display LCD y todos los LEDs si la variable "encendido" es false
			LcdItsE0803Off();
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}

		// Esperar el tiempo especificado antes de medir de nuevo
		vTaskDelay(DELAY_MEDIR / portTICK_PERIOD_MS);
	}
}

 /**
  * @brief Tarea encargada de leer las teclas y cambiar el estado de la variable encendido
  * y la variable Hold
  */
static void LeerTeclas(void *pvParameter)
{
	uint8_t teclas;
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

/**
 * @brief Tarea encargada de mostrar en el display LCD la distancia medida
 * por el sensor HC-SR04. La tarea tambien se encarga de controlar los LEDs
 * según la distancia medida.

 */
static void MostrarDisplayLCD(void *pvParameter)
{
	while (true)
	{

		if(encendido){

			if(!hold){
				LcdItsE0803Write(distancia);

			}
			ControlarLEDs();
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
