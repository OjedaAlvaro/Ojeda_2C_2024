/*! @mainpage P2_ejercicio4_osciloscopio
 *
 * @section genDesc General Description
 *
 * Este programa digitaliza una señal analógica y la transmite a un graficador de puerto serie de la PC. 
 * Además permite convertir una señal digital en una señal analógica y visualizarla con un graficador de puerto serie de la PC. 
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
 * | 25/09/2024 | Document creation		                         |
 *
 * @author Ojeda Alvaro
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/** @brief tamaño del arreglo que modela el ecg */
#define BUFFER_SIZE 231

/** @brief Periodo de muestreo para el conversor AD en microsegundos */
#define DELAY_MUESTREO_AD 2 * 1000

/** @brief Periodo de muestreo para el conversor DA en microsegundos */
#define DELAY_MUESTREO_DA 4 * 1000


/*==================[internal data definition]===============================*/


TaskHandle_t muestreo_task_handle = NULL;
TaskHandle_t main_task_handle = NULL;

/** @brief Arreglo que modela el ecg */
const char ecg[BUFFER_SIZE] = {
	76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/** @brief Arreglo que modela un segundo  ecg */
unsigned char ecg_2[] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;

/*==================[internal functions declaration]=========================*/
/** @brief Timer que envia una notificaciones cada 2 mseg a conversionAD
 * @return void
*/
void FuncTimerMuestreoAD(void *param)
{
	vTaskNotifyGiveFromISR(muestreo_task_handle, pdFALSE);
}

/** @brief Timer que envia una notificaciones cada 4 mseg a conversionDA
 * @return void
*/
void FuncTimerMuestreoDA(void *param)
{
	vTaskNotifyGiveFromISR(main_task_handle, pdFALSE);
}

/**
 * @brief  Lee la señal analgica y la envia a la PC por puerto serie en codigo ASCII
 * @param pvParameter puntero a un void
 * @return void
 */
static void ConversionAD(void *pvParameter)
{
	uint16_t senial_analogica = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		/* se lee el valor de la senial analogica */
		AnalogInputReadSingle(CH1, &senial_analogica);

		/* se envia la señal a la pc */
		UartSendString(UART_PC, (char *)UartItoa(senial_analogica, 10));
		UartSendString(UART_PC, "\r\n");
	}
}

/**
 * @brief Convierte una señal digital en una señal analógica y la envia a la PC por puerto serie
 * @param pvParameter puntero a un void
 * @return void
 */
static void ConversionDA(void *pvParameter)
{
	uint8_t contador = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (contador < BUFFER_SIZE)
		{
			/**
			 * Se escribe el valor de la señal analógica en el puerto de salida analógica
			 * se utiliza el arreglo ecg definido en la parte superior del archivo
			 */
			AnalogOutputWrite(ecg[contador]);
			//AnalogOutputWrite(ECG_2[contador]); Prueba de segundo ecg
			contador++;
		}
		else if (contador == BUFFER_SIZE)
		{
			/**
			 * se reinicia el contador para que comience de nuevo en la posición 0 del arreglo
			 */
			contador = 0;
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	analog_input_config_t config_senial_analog = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0

	};

	AnalogInputInit(&config_senial_analog);
	AnalogOutputInit();

	timer_config_t timer_muestreo_AD = {
		.timer = TIMER_A,
		.period = DELAY_MUESTREO_AD,
		.func_p = FuncTimerMuestreoAD,
		.param_p = NULL};
	TimerInit(&timer_muestreo_AD);

	timer_config_t timer_muestreo_DA = {
		.timer = TIMER_B,
		.period = DELAY_MUESTREO_DA,
		.func_p = FuncTimerMuestreoDA,
		.param_p = NULL};
	TimerInit(&timer_muestreo_DA);

	serial_config_t uart_config = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&uart_config);

	xTaskCreate(ConversionAD, "LeerDatoSenialAnalogica", 1024, NULL, 5, &muestreo_task_handle);
	xTaskCreate(ConversionDA, "Escritura de ECG", 2048, NULL, 5, &main_task_handle);
	
	TimerStart(timer_muestreo_AD.timer);
	TimerStart(timer_muestreo_DA.timer);
}

/*==================[end of file]============================================*/
