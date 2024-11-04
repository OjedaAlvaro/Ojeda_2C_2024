/*! @mainpage Examen Electronica programable
 *
 * @section genDesc General Description
 *
 * Examen Individual 04-11-24
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que permita detectar
 * eventos peligrosos para ciclistas.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * |   TRIG	 	    | 	GPIO_2		|
 * | acelerometro x	| 	CH1		    |
 * | acelerometro y | 	CH2		    |
 * | acelerometro z |   CH3		    |
 * |   Buzzer	 	| 	GPIO_10		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
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
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief Variable global que almacena la distancia medida por el HC-SR04
 */
uint16_t distancia = 0;
/**
 * @brief variable booleana que se activa cuando hay que tener precaucion
 */
bool precaucion = false;
/**
 * @brief Variable booleana que se activa cuando hay peligro
 */
bool peligro = false;
/**
 * @brief Variable que almacena la medicion en el eje x del acelerometro
 */
uint16_t ejeX = 0;

/**
 * @brief Variable que almacena la medicion en el eje y del acelerometro
 */
uint16_t ejeY = 0;

/**
 * @brief Variable que almacena la medicion en el eje z del acelerometro
 */
uint16_t ejeZ = 0;

/** @brief GPIO donde se conecta el buzzer */
#define GPIO_BUZZER GPIO_10

/** @brief periodo de muestreo para la tarea de medicion */
#define DELAY_MEDIR_uS 500 * 1000 // 500ms

/** @brief periodo de muestreo para la tarea del acelerometro */
#define DELAY_ACELEROMETRO 10 * 1000 // 10ms

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t acelerometro_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/** @brief Envía una notificación a la tarea asociada  la medicion de distancia */
void FuncTimerMedir(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE); 
}

/** @brief Envía una notificación a la tarea asociada a la medicion del acelerometro */
void FuncTimerAcelerometro(void *param)
{
	vTaskNotifyGiveFromISR(acelerometro_task_handle, pdFALSE); 
}
/**
 * @brief Controla los LEDs segun la distancia medida por el HC-SR04 y cambia el estado de precaucion y peligro
 */
void ControlarLEDs(void)
{
	// Tiene que fijarse que leds prender y apagar
	if (distancia > 500)
	{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		peligro = false;
		precaucion = false;
	}
	else if (distancia < 500 && distancia > 300)
	{
		precaucion = true;
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else if (distancia < 300)
	{
		peligro = true;
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
	else
	{
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
}
/**
 * @brief Controla el buzzer en estado de precaucion
 */
void ControlBuzzerPrecaucion(void){
	GPIOOn(GPIO_BUZZER);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	GPIOff(GPIO_BUZZER);
}

/**
 * @brief Controla el buzzer en estado de peligro */
void ControlBuzzerPeligro(void){
	GPIOOn(GPIO_BUZZER);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	GPIOff(GPIO_BUZZER);
	
}
/**
 * @brief guarda el valor medido y llama a controlar LEDS
 */
void MedirDistancia(void)
{
	distancia = HcSr04ReadDistanceInCentimeters();
	ControlarLEDs();
}
/**
 * @brief Tarea encargada de medir distancia y enviar mensajes via UART
 */
static void tareaMedirDistancia(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		MedirDistancia();
		if (precaucion)
		{	
			ControlBuzzerPrecaucion();
			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca");
			UartSendString(UART_CONNECTOR, "r\n");
		}
		else if (peligro)
		{	
			ControlBuzzerPeligro();
			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca");
			UartSendString(UART_CONNECTOR, "r\n");
		}
	}
}
/**
 * @brief Tarea encargada detectar caidas y enviar mensajes via UART
 */
static void tareaAcelerometro(void *pvParameter)
{	
	float suma=0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &ejeX);
		AnalogInputReadSingle(CH2, &ejeY);
		AnalogInputReadSingle(CH3, &ejeZ);

		ejeX = (ejeX/0.3)-1.65;
		ejeY = (ejeY/0.3)-1.65;
		ejeZ = (ejeZ/0.3)-1.65;
		suma= ejeX+ejeY+ejeZ;

		if (suma> 4){
			UartSendString(UART_CONNECTOR, "Caída detectada");
			UartSendString(UART_CONNECTOR, "r\n");
		}



	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	GPIOInit(GPIO_BUZZER, GPIO_OUTPUT);
	HcSr04Init(GPIO_3, GPIO_2);

	analog_input_config_t config_ejeX = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0

	};

	analog_input_config_t config_ejeY = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0

	};
	analog_input_config_t config_ejeZ = {
		.input = CH3,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0

	};

	AnalogInputInit(&config_ejeX);
	AnalogInputInit(&config_ejeY);
	AnalogInputInit(&config_ejeZ);

	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = DELAY_MEDIR_uS,
		.func_p = FuncTimerMedir,
		.param_p = NULL};
	TimerInit(&timer_medir);

	timer_config_t timer_acel = {
		.timer = TIMER_B,
		.period = DELAY_ACELEROMETRO,
		.func_p = FuncTimerAcelerometro,
		.param_p = NULL};
	TimerInit(&timer_acel);

	xTaskCreate(tareaMedirDistancia, "Tarea distancia", 1024, NULL, 5, &medir_task_handle);
	xTaskCreate(tareaAcelerometro, "Tarea Acelerometro", 1024, NULL, 5, &acelerometro_task_handle);


	TimerStart(timer_medir.timer);
	TimerStart(timer_acel.timer);
}
/*==================[end of file]============================================*/