#include "stm32f4xx.h"

int main(void)
{
	while(1)
	{

	}
}

int ClockInit(void)
{
	__IO int StartUpCounter;

	//ЗАПУСК КВАРЦЕВОГО ГЕНЕРАТОРА:

	RCC->CR |= (1<<RCC_CR_HSEON_Pos); //Запускаем генератор HSE

	//Ждем успешного запуска или окончания тайм-аута
	for(StartUpCounter = 0; ; StartUpCounter++)
	{
		//Если успешно запустилось, то выходим из цикла
		if(RCC->CR & (1<<RCC_CR_HSERDY_Pos))
		{
			break;
		}
		//Если не запустилось, то отключаем все, что включили и возвращаем ошибку
		if(StartUpCounter > 0x1000)
		{
			RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
			return 1;
		}
	}

	//НАСТРОЙКА И ЗАПУСК PLL:
	//Частота кварца HSE_VALUE = 25 MHz
	//f_{PLL general clock output} = [(HSE_VALUE/PLLM)*PLLN]/PLLP

	//Устанавливаем PLLM = 25 <---> (01 1001)
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_4);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_5);

	//Устанавливаем PLLN = 144 <---> (0 1001 0000)
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLN_7);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_0 | RCC_PLLCFGR_PLLN_1 | RCC_PLLCFGR_PLLN_2 | RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_5 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_8);

	//Устанавливаем PLLP = 2 <---> (00)
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP_0 | RCC_PLLCFGR_PLLP_1);

	RCC->PLLCFGR |= (1<<RCC_PLLCFGR_PLLSRC_Pos);   //Тактирование PLL от HSE

	RCC->CR |= (1<<RCC_CR_PLLON_Pos);              //Запускаем PLL

	//Ждем успешного запуска или окончания тайм-аута
	for(StartUpCounter=0; ; StartUpCounter++)
	{
		if(RCC->CR & (1<<RCC_CR_PLLRDY_Pos))
			break;

		if(StartUpCounter > 0x1000)
		{
			RCC->CR &= ~(1<<RCC_CR_HSEON_Pos);
			RCC->CR &= ~(1<<RCC_CR_PLLON_Pos);
			return 2;
		}
	}

	// НАСТРОЙКА FLASH И ДЕЛИТЕЛЕЙ:

	//Устанавливаем 2 цикла ожидания для Flash
	FLASH->ACR |= (0x02<<FLASH_ACR_LATENCY_Pos);

	RCC->CFGR |= (0x00<<RCC_CFGR_PPRE2_Pos) //Делитель шины APB2 равен 1
		| (0x04<<RCC_CFGR_PPRE1_Pos)        //Делитель шины APB1 равен 2
		| (0x00<<RCC_CFGR_HPRE_Pos);        //Делитель AHB равен 1


	RCC->CFGR |= (0x02<<RCC_CFGR_SW_Pos);   //Переключаемся на работу от PLL

	//Ждем, пока переключимся
	while((RCC->CFGR & RCC_CFGR_SWS_Msk) != (0x02<<RCC_CFGR_SWS_Pos))
	{

	}

	//После того, как переключились на внешний источник такирования отключаем внутренний RC-генератор (HSI) для экономии энергии
	RCC->CR &= ~(1<<RCC_CR_HSION_Pos);

	return 0;
}
