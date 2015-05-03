/* This header is only included internally. */

/* This header is only included internally. */
#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_WWDG_USER_IRQ))
IRQ_VEC_N_OP(0)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_PVD_USER_IRQ))
IRQ_VEC_N_OP(1)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TAMP_STAMP_USER_IRQ))
IRQ_VEC_N_OP(2)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_RTC_WKUP_USER_IRQ))
IRQ_VEC_N_OP(3)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_FLASH_USER_IRQ))
IRQ_VEC_N_OP(4)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_RCC_USER_IRQ))
IRQ_VEC_N_OP(5)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI0_USER_IRQ))
IRQ_VEC_N_OP(6)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI1_USER_IRQ))
IRQ_VEC_N_OP(7)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI2_USER_IRQ))
IRQ_VEC_N_OP(8)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI3_USER_IRQ))
IRQ_VEC_N_OP(9)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI4_USER_IRQ))
IRQ_VEC_N_OP(10)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream0_USER_IRQ))
IRQ_VEC_N_OP(11)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream1_USER_IRQ))
IRQ_VEC_N_OP(12)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream2_USER_IRQ))
IRQ_VEC_N_OP(13)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream3_USER_IRQ))
IRQ_VEC_N_OP(14)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream4_USER_IRQ))
IRQ_VEC_N_OP(15)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream5_USER_IRQ))
IRQ_VEC_N_OP(16)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA_Stream6_USER_IRQ))
IRQ_VEC_N_OP(17)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_ADC_USER_IRQ))
IRQ_VEC_N_OP(18)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN1_TX_USER_IRQ))
IRQ_VEC_N_OP(19)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN1_RX_USER_IRQ))
IRQ_VEC_N_OP(20)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN1_RX1_USER_IRQ))
IRQ_VEC_N_OP(21)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN1_SCE_USER_IRQ))
IRQ_VEC_N_OP(22)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI9_5_USER_IRQ))
IRQ_VEC_N_OP(23)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_BRK_USER_IRQ))
IRQ_VEC_N_OP(24)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_UP_USER_IRQ))
IRQ_VEC_N_OP(25)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_TRG_COM_USER_IRQ))
IRQ_VEC_N_OP(26)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_CC_USER_IRQ))
IRQ_VEC_N_OP(27)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM2_USER_IRQ))
IRQ_VEC_N_OP(28)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM3_USER_IRQ))
IRQ_VEC_N_OP(29)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM4_USER_IRQ))
IRQ_VEC_N_OP(30)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C1_EV_USER_IRQ))
IRQ_VEC_N_OP(31)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C1_ER_USER_IRQ))
IRQ_VEC_N_OP(32)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C2_EV_USER_IRQ))
IRQ_VEC_N_OP(33)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C2_ER_USER_IRQ))
IRQ_VEC_N_OP(34)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_SPI1_USER_IRQ))
IRQ_VEC_N_OP(35)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_SPI2_USER_IRQ))
IRQ_VEC_N_OP(36)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_USART1_USER_IRQ))
IRQ_VEC_N_OP(37)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_USART2_USER_IRQ))
IRQ_VEC_N_OP(38)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_USART3_USER_IRQ))
IRQ_VEC_N_OP(39)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_EXTI15_10_USER_IRQ))
IRQ_VEC_N_OP(40)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_RTC_Alarm_USER_IRQ))
IRQ_VEC_N_OP(41)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_FS_WKUP_USER_IRQ))
IRQ_VEC_N_OP(42)
#endif
