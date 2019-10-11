extern "C" {
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

#include <stdio.h>

int __io_putchar(int ch) {

	uint8_t character = (uint8_t) ch;
	HAL_UART_Transmit(&huart2, &character, 1, 1000);
	return ch;
}
}

void waitForEnter(void) {
	char inputchar = 0;

	while (inputchar != '\r')
		HAL_UART_Receive(&huart2, (uint8_t*) &inputchar, 1, 0);

}

#include "WAL_Libraries/linear_scale.h"

void SystemClock_Config(void);

uint32_t ADC_RAW[100];
float adc_raw = 0;
bool print_data = false;

linear_scale sensorA(1000, 2000, 0, 100);
linear_scale sensorB(1000, 2000, 0, 100);

__attribute__((__section__(".user_data")))      uint8_t calibrated;
__attribute__((__section__(".user_data")))      uint32_t caldataA[4];
__attribute__((__section__(".user_data")))      uint32_t caldataB[4];

int main(void) {

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();

	HAL_ADC_Start_DMA(&hadc1, ADC_RAW, 100);

	volatile float testval = 0;
	volatile float fullval = 0;
	volatile float noval = 0;
	printf(">>\r\n");
	printf(">>\r\n");
	printf(">>\r\n");

	if (calibrated == 1) {
		float cala_oldA[4] = { caldataA[0] / 100000, caldataA[1] / 100000,
				caldataA[2] / 100000, caldataA[3] / 100000 };
		sensorA.setCalibration((float*) cala_oldA);
		printf(">>\r\n");
		printf("Calibration data A: %d, %d, %d, %d\n\r",
				(int) caldataA[0] / 100000, (int) caldataA[1] / 100000,
				(int) caldataA[2] / 100000, (int) caldataA[3] / 100000);
//
//		float cala_oldB[4] = { caldataB[0] / 100000, caldataB[1] / 100000,
//				caldataB[2] / 100000, caldataB[3] / 100000 };
//		sensorB.setCalibration((float*) cala_oldB);
//		printf(">>\r\n");
//		printf("Calibration data B: %d, %d, %d, %d\n\r",
//				(int) caldataB[0] / 100000, (int) caldataB[1] / 100000,
//				(int) caldataB[2] / 100000, (int) caldataB[3] / 100000);

	}


	while (1) {

		char inputchar = 0;
		HAL_UART_Receive(&huart2, (uint8_t*) &inputchar, 1, 0);

		if (inputchar != 0) {
			switch (inputchar) {
			case 'c':
				printf("Full Throttle\n\r");
				waitForEnter();
				sensorA.calibrate_max(adc_raw, 100);

				printf("No Throttle\n\r");
				waitForEnter();
				sensorA.calibrate_min(adc_raw, 0);

				float newcalibration[4];

//				uint64_t addr8 = (uint32_t)&caldataA[0];

				sensorA.getCalibration(newcalibration);

				HAL_FLASH_Unlock();
				__HAL_FLASH_CLEAR_FLAG(
						FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

				FLASH_Erase_Sector(FLASH_SECTOR_7, VOLTAGE_RANGE_3);
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
						(uint32_t) &calibrated, 1);

				for (int i = 0; i < 4; i++)
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
							(uint32_t) &caldataA[i],
							(uint32_t) (newcalibration[i]) * 100000);
				HAL_FLASH_Lock();

				printf("Calibration Data: %d, %d, %d, %d",
						(int) newcalibration[0], (int) newcalibration[1],
						(int) newcalibration[2], (int) newcalibration[3]);
				printf("Done!\n\r");
				printf(">>");

				break;

			case 'd':
				print_data = true;
				break;

			case 's':
				print_data = false;
				break;

			default:
				printf("Usage: \n\r");
				printf("\t 'c' Calibrate Throttle\n\r");
				printf("\t 'd' Display Data\n\r");
				printf("\t 's' Stop Displaying Data\n\r");
				printf(">>");

			}
		}

		if (print_data)
			printf("adc: %d sensorA value: %d \n\r", (int) adc_raw,
					(int) sensorA.getValue());

		HAL_Delay(100);

	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	adc_raw = 0;
	for (int i = 0; i < 100; i++)
		adc_raw = adc_raw + ADC_RAW[i];
	adc_raw = adc_raw / 100;

	sensorA.scale(adc_raw);

}

void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	/**Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE()
	;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK
	;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void Error_Handler(void) {

	while (1) {
	}

}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{

//     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}
#endif
