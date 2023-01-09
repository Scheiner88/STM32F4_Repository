#include "main.h"
#include "stm32f4xx.h"

uint8_t tim2_count;

int ClockInit(void);
static void TIM2_Init(void);
static void MX_GPIO_Init(void);

int main(void)
{
	ClockInit();
	TIM2_Init();
	MX_GPIO_Init();

	tim2_count = 0;
	LL_TIM_EnableIT_UPDATE(TIM2);
	LL_TIM_EnableCounter(TIM2);
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
	//Частота кварца 8 MHz
	//f_{PLL general clock output} = [(HSE_VALUE/PLLM)*PLLN]/PLLP

	//Устанавливаем PLLM = 8 <---> (00 1000)
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_3);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_5 | RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLM_4);

	//Устанавливаем PLLN = 144 <---> (0 1001 0000)
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLN_7);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_0 | RCC_PLLCFGR_PLLN_1 | RCC_PLLCFGR_PLLN_2 | RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_5 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_8);

	//Устанавливаем PLLP = 2 <---> (00)
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP_0 | RCC_PLLCFGR_PLLP_1);

	RCC->PLLCFGR |= (1<<RCC_PLLCFGR_PLLSRC_Pos);   //Тактирование PLL от HSE

	RCC->CR |= (1<<RCC_CR_PLLON_Pos);              //Запускаем PLL

	//Ждем успешного запуска или окончания тайм-аута
	for(StartUpCounter = 0; ; StartUpCounter++)
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

static void TIM2_Init(void)
{
	  LL_TIM_InitTypeDef TIM_InitStruct = {0};

	  /* Peripheral clock enable */
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

	  /* TIM2 interrupt Init */
	  NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	  NVIC_EnableIRQ(TIM2_IRQn);

	  TIM_InitStruct.Prescaler = 7200;
	  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	  TIM_InitStruct.Autoreload = 10000;
	  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	  LL_TIM_Init(TIM2, &TIM_InitStruct);
	  LL_TIM_DisableARRPreload(TIM2);
	  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
	  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);
	  LL_TIM_DisableMasterSlaveMode(TIM2);
}

static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);

  /**/
  LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_13|LL_GPIO_PIN_14);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

void TIM2_Callback(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2))
	{
		LL_TIM_ClearFlag_UPDATE(TIM2);
	    switch(tim2_count)
	    {
	      case 0:
	    	  LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_13);
	    	  LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_14);
	    	  break;
	      case 1:
	    	  LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
	    	  LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_13);
	    	  break;
	    }
		tim2_count++;
		if(tim2_count>1) tim2_count = 0;
	}
}
