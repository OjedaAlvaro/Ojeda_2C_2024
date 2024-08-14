/*! @mainpage Ejercicios 4, 5 y 6
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
 * | 14/08/2024 | Document creation		                         |
 *
 * @author Alvaro Ojeda
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	int8_t salida=0;
	for (int i = digits - 1; i >= 0; i--) {

		
        bcd_number[i] = data % 10;            // Extrae el dígito menos significativo
        data /= 10;     
		                                
    }
	return salida;
	
}
void codificarDigito(uint8_t n_decimal,gpioConf_t *param_gpio){
	// Convertir el dígito decimal a binario y almacenarlo en el arreglo
	uint8_t arreglo_binario[4];
    for (int i = 0; i <4; i++) {
        arreglo_binario[i] = n_decimal % 2;
        n_decimal /= 2;
    }

	//fijarme con mascaras el estado de los GPIO
	for(int j=0;j<4;j++)
	{
		if(arreglo_binario[j])
		{
			GPIOOn(param_gpio[j].pin);
		}
		else{
			GPIOOff(param_gpio[j].pin);
		}
	}


	
}

void codificarMasDigitos(uint32_t numero,uint8_t cantDigitos,gpioConf_t *valor,gpioConf_t *pos){
	

}


/*==================[external functions definition]==========================*/
void app_main(void){
	//Ejercicio 4
	int8_t arreglo[4];
	convertToBcdArray(2465,4,&arreglo);
	//printf("Numero en BCD:%d %d %d %d",arreglo[0],arreglo[1],arreglo[2],arreglo[3]);
	
	//Ejercicio 5
	GPIOInit(GPIO_20,GPIO_OUTPUT);
	GPIOInit(GPIO_21,GPIO_OUTPUT);
	GPIOInit(GPIO_22,GPIO_OUTPUT);
	GPIOInit(GPIO_23,GPIO_OUTPUT);

	gpioConf_t arregloGPIO[]=
	{
		{GPIO_20,GPIO_OUTPUT},
		{GPIO_21,GPIO_OUTPUT},
		{GPIO_22,GPIO_OUTPUT},
		{GPIO_23,GPIO_OUTPUT}

	};

	gpioConf_t Posicion_digitos[]=
	{
		{GPIO_19,GPIO_OUTPUT},
		{GPIO_18,GPIO_OUTPUT},
		{GPIO_9,GPIO_OUTPUT}

	};
	
	codificarDigito(1,arregloGPIO);

	
}




/*==================[end of file]============================================*/