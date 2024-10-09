/*! @mainpage P2_ejercicio4_osciloscopio
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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

/*==================[internal data definition]===============================*/
#define DELAY_MUESTREO_uS 2*1000

TaskHandle_t muestreo_task_handle = NULL;

uint16_t senial_analogica = 0;

/*==================[internal functions declaration]=========================*/
void FuncTimerMuestreo(void *param)
{
	vTaskNotifyGiveFromISR(muestreo_task_handle, pdFALSE); 
}

static void LeerSenialAnalog(void *pvParameter){
	while (true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &senial_analogica);
		UartSendString(UART_PC,(char*)UartItoa(senial_analogica,10));
		UartSendString(UART_PC,"\r\n");
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){

	analog_input_config_t config_analogico = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	
	};

	AnalogInputInit(&config_analogico);

	timer_config_t timer_muestreo = {
		.timer = TIMER_A,
		.period = DELAY_MUESTREO_uS,
		.func_p = FuncTimerMuestreo,
		.param_p = NULL};
	TimerInit(&timer_muestreo);	

	serial_config_t uart_config = {
	.port = UART_PC,
	.baud_rate = 9600,
	.func_p = NULL,
	.param_p = NULL
	};
		
	UartInit(&uart_config);	

	xTaskCreate(LeerSenialAnalog, "LeerDatoSenialAnalog", 512, NULL, 5, &muestreo_task_handle);	
	TimerStart(&timer_muestreo);
} 

/*==================[end of file]============================================*/
