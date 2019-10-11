extern "C" {
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
}

#include "WAL_Libraries/linear_scale.h"

int __io_putchar(int ch) {

	uint8_t character = (uint8_t) ch;
	HAL_UART_Transmit(&huart2, &character, 1, 1000);
	return ch;
}


void SystemClock_Config(void);
uint32_t ADC_RAW[200];

int main(void) {

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_USART1_UART_Init();
	MX_CAN_Init();

	HAL_ADCEx_Calibration_Start(&hadc);
	HAL_ADC_Start_DMA(&hadc, ADC_RAW, 200);

	while (1) {

	}

}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	volatile float adc0raw = 0;
	volatile float adc1raw = 0;

	for (int i = 0; i < 200; i = i + 2)
		adc0raw = adc0raw + ADC_RAW[i];

	for (int i = 1; i < 200; i = i + 2)
		adc1raw = adc1raw + ADC_RAW[i];

	adc0raw = adc0raw / 100;
	adc1raw = adc1raw / 100;



}

void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

void Error_Handler(void) {

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char
		{

			tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)

		}
#endif

