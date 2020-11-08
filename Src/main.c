/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
void proccesDmaData(uint8_t* sign, uint8_t len);
void calculateLetters(uint8_t total_len);


/* Space for your global variables. */
uint8_t count = 0;
uint8_t tx_data[] = "";
 uint8_t rx_data[35];
 letter_count_ letter_count;

	// type your global variables here:


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  /* Space for your local variables, callback registration ...*/
  USART2_RegisterCallback(proccesDmaData);
  	  //type your code here:

  while (1)
  {
	  /* Periodic transmission of information about DMA Rx buffer state.
	   * Transmission frequency - 5Hz.
	   * Message format - "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %]: %f%"
	   * Example message (what I wish to see in terminal) - Buffer capacity: 1000 bytes, occupied memory: 231 bytes, load [in %]: 23.1%
	   */
	#if POLLING
		//Polling for new data, no interrupts
		USART2_CheckDmaReception();
		LL_mDelay(10);
	#else
		//USART2_PutBuffer(tx_data, sizeof(tx_data));
		//LL_mDelay(1000);
	#endif
  	  	  	  //type your code here:
  }
  /* USER CODE END 3 */
}


void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
}

/*
 * Implementation of function processing data received via USART.
 */
void proccesDmaData(uint8_t* sign, uint8_t len)
{
	/* Process received data */

	static uint8_t total_len = 0; //number
	static uint8_t first_symbol = 0; //boolean
	static uint8_t last_symbol = 0; //boolean
	static uint8_t first_symbol_index = 0; //number
	static uint8_t first_symbol_cycle = 0; //boolean

	//toto sa deje zakazdym ked sme este nedostali znak '#'
	//program skontroluje teda prijate data ci sa tam nachadza
	//ak nie tak sa nic nestane a program nespravi nic
	if(first_symbol == 0){
		for(uint8_t i = 0; i < len; i++){
				//ak najde prvy znak flipne boolean, zapamata si jeho index a zapisuje dalsie data uz do rx_data pola
		    	if(*(sign+i) == '#'){
		    		first_symbol = 1;
		    		first_symbol_index = i;
		    		first_symbol_cycle = 1;
		    	}
		    	//tuto zacne zapisovat data do rx_data ak nasiel prvy znak
		    	if(first_symbol == 1){
		    		//ak by som dovrsil 35 prijatych znakov, prestane zapisovat a posle data na vyhodnotenie
		    		if(i == 35){
		    			last_symbol = 1;
		    			total_len = 35;
		    			break;
		    		}
		    		rx_data[i-first_symbol_index] = *(sign+i);
		    		//ak najde aj koncovy znak, flipne aj boolean koncoveho znaku a breakne for cyklus
		    		if(*(sign+i) == '$'){
		    			last_symbol = 1;
		    			total_len = i - first_symbol_index;
		    			break;
		    		}
		    		total_len = first_symbol_index;
		    	}
		}
	}

	//ak program nasiel prvy znak ale nenasiel v tom istom stringu aj koncovi stane sa toto
	if(first_symbol == 1 && first_symbol_cycle == 0){

	    for(uint8_t i = 0; i < len; i++)
	    {
	    	//ak by som dovrsil 35 prijatych znakov, prestane zapisovat a posle data na vyhodnotenie
	    	if(total_len + i == 35){
	    		total_len = 35;
	    		last_symbol = 1;
	    		break;
	    	}
	    	//klasika, len pozeram ci nemam koncovy znak a zapisujem popri tom data do rx_data
	    	rx_data[i+total_len]=*(sign+i);
	    	if(*(sign+i) == '$'){
	    		last_symbol = 1;
	    		total_len += i;
	    		break;
	    	}
	    }
	    total_len += len;

	}

	//toto je len toggle na to aby sme vedeli ci sme v cykle spracovania stringu v ktorom sa nasiel prvy znak
	//ak by pouzivatel poslal koncovy znak v dalsom stringu
	if(first_symbol_cycle == 1){
		first_symbol_cycle = 0;
	}

	if(last_symbol == 1){
		calculateLetters(total_len);
		for(uint8_t i = 0;i<35;i++){
			rx_data[i] = 0;
		}
		total_len = 0;
		last_symbol = 0;
		first_symbol = 0;
	}


		// type your algorithm here:
}

void calculateLetters(uint8_t total_len){
	for(uint8_t i=0;i<total_len;i++){
				if(rx_data[i] > 96 && rx_data[i] < 123){
					letter_count.small_letter++;
				}
				if(rx_data[i] > 64 && rx_data[i]<91){
					letter_count.capital_letter++;
				}
			}
}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
