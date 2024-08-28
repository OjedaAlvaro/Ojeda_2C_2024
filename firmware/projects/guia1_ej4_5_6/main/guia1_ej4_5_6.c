/*! @mainpage Ejercicios 4, 5 y 6
 *
 * @section genDesc General Description
 *
 * Con estos ejercicios podremos mostrar numeros en una pantalla LCD usando la ESP32
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

/*!
 * @def ON
 * @brief LED encendido 
 */
#define ON 1
/*!
 * \def OFF
 * \brief LED apagado 
 */
#define OFF 0


/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un numero decimal en BCD y lo guarda en un arreglo
 * @param data Decimal a convertir
 * @param digitos cantidad de digitos del numero decimal
 * @param bcd_number Arreglo donde se va a guardar el numero en BCD
 */
void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	
	for (int i = digits - 1; i >= 0; i--) {

		
        bcd_number[i] = data % 10;            // Extrae el dígito menos significativo
        data /= 10;     
		                                
    }
	
}

/**
 * @brief cambia el estado de cada GPIO según el estado del bit correspondiente en el BCD ingresado.
 * @param n_decimal numero que va ser representado
 * @param param_gpio estructura que contiene la configuración de los puertos
 */
void codificarDigito(uint8_t n_decimal,gpioConf_t *param_gpio){
	// Convertir el dígito decimal a binario y almacenarlo en el arreglo
	uint8_t arreglo_binario[4];
    for (int i = 0; i <4; i++) {
        arreglo_binario[i] = n_decimal % 2;
        n_decimal /= 2;
    }

	//fijarme recorriendo arreglo_binario que led encender
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

/**
 * @brief Codifica un numero de mas de 1 cifras en un dislay LCD
 * @param numero numero que va ser representado
 * @param cantDigitos cantidad de digitos a usar del display
 * @param valor estructura que contiene la configuración de los puertos
 * @param pos estructura que mapea los pines que dan el peso del digito
 */

void codificarMasDigitos(uint32_t numero,uint8_t cantDigitos,gpioConf_t *valor,gpioConf_t *pos){

	uint8_t numero_separado[cantDigitos];
	convertToBcdArray(numero,cantDigitos,numero_separado);
	for(int indice=0;indice<3;indice++){
		//Ahora escribo el valor
		codificarDigito(numero_separado[indice],valor);
		//Con esto hago el pulso
		GPIOOn(pos[indice].pin);
		GPIOOff(pos[indice].pin);
		
		
	}

	

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

	//codificarDigito(1,arregloGPIO);

	//Ejercicio 6
	gpioConf_t posicion_digitos[]=
	{
		{GPIO_19,GPIO_OUTPUT},
		{GPIO_18,GPIO_OUTPUT},
		{GPIO_9,GPIO_OUTPUT}

	};
	//Inicializo los puertos
	GPIOInit(posicion_digitos[0].pin,posicion_digitos[0].dir);
	GPIOInit(posicion_digitos[1].pin,posicion_digitos[1].dir);
	GPIOInit(posicion_digitos[2].pin,posicion_digitos[2].dir);
	
	
	codificarMasDigitos(51,3,arregloGPIO,posicion_digitos);

	
}


/*==================[end of file]============================================*/
