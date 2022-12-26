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

	RCC->CFGR |= (0x07<<RCC_PLLCFGR_PLLN_Pos)      ////PLL множитель. Нужно заменить 0x07 на что-нибудь осмысленное (в документации говорят можно задавать 9ти битное число)
		| (0x01<<RCC_PLLCFGR_PLLSRC_Pos);          //Тактирование PLL от HSE

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

	/* НАСТРОЙКА FLASH И ДЕЛИТЕЛЕЙ (не готово):

	FLASH->ACR |= (0x02<<FLASH_ACR_LATENCY_Pos);

	RCC->CFGR |= (0x00<<RCC_CFGR_PPRE2_Pos)
		| (0x04<<RCC_CFGR_PPRE1_Pos)
		| (0x00<<RCC_CFGR_HPRE_Pos);


	RCC->CFGR |= (0x02<<RCC_CFGR_SW_Pos);

	while((RCC->CFGR & RCC_CFGR_SWS_Msk) != (0x02<<RCC_CFGR_SWS_Pos))
	{

	}

	RCC->CR &= ~(1<<RCC_CR_HSION_Pos);

	return 0;
	*/
}
