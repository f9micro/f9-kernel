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
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream0_USER_IRQ))
IRQ_VEC_N_OP(11)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream1_USER_IRQ))
IRQ_VEC_N_OP(12)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream2_USER_IRQ))
IRQ_VEC_N_OP(13)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream3_USER_IRQ))
IRQ_VEC_N_OP(14)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream4_USER_IRQ))
IRQ_VEC_N_OP(15)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream5_USER_IRQ))
IRQ_VEC_N_OP(16)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream6_USER_IRQ))
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
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_BRK_TIM9_USER_IRQ))
IRQ_VEC_N_OP(24)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_UP_TIM10_USER_IRQ))
IRQ_VEC_N_OP(25)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM1_TRG_COM_TIM11_USER_IRQ))
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

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM8_BRK_TIM12_USER_IRQ))
IRQ_VEC_N_OP(43)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM8_UP_TIM13_USER_IRQ))
IRQ_VEC_N_OP(44)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM8_TRG_COM_TIM14_USER_IRQ))
IRQ_VEC_N_OP(45)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM8_CC_USER_IRQ))
IRQ_VEC_N_OP(46)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA1_Stream7_USER_IRQ))
IRQ_VEC_N_OP(47)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_FSMC_USER_IRQ))
IRQ_VEC_N_OP(48)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_SDIO_USER_IRQ))
IRQ_VEC_N_OP(49)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM5_USER_IRQ))
IRQ_VEC_N_OP(50)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_SPI3_USER_IRQ))
IRQ_VEC_N_OP(51)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_UART4_USER_IRQ))
IRQ_VEC_N_OP(52)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_UART5_USER_IRQ))
IRQ_VEC_N_OP(53)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM6_DAC_USER_IRQ))
IRQ_VEC_N_OP(54)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_TIM7_USER_IRQ))
IRQ_VEC_N_OP(55)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream0_USER_IRQ))
IRQ_VEC_N_OP(56)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream1_USER_IRQ))
IRQ_VEC_N_OP(57)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream2_USER_IRQ))
IRQ_VEC_N_OP(58)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream3_USER_IRQ))
IRQ_VEC_N_OP(59)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream4_USER_IRQ))
IRQ_VEC_N_OP(60)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_ETH_USER_IRQ))
IRQ_VEC_N_OP(61)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_ETH_WKUP_USER_IRQ))
IRQ_VEC_N_OP(62)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN2_TX_USER_IRQ))
IRQ_VEC_N_OP(63)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN2_RX0_USER_IRQ))
IRQ_VEC_N_OP(64)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN2_RX1_USER_IRQ))
IRQ_VEC_N_OP(65)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CAN2_SCE_USER_IRQ))
IRQ_VEC_N_OP(66)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_FS_USER_IRQ))
IRQ_VEC_N_OP(67)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream5_USER_IRQ))
IRQ_VEC_N_OP(68)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream6_USER_IRQ))
IRQ_VEC_N_OP(69)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DMA2_Stream7_USER_IRQ))
IRQ_VEC_N_OP(70)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_USART6_USER_IRQ))
IRQ_VEC_N_OP(71)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C3_EV_USER_IRQ))
IRQ_VEC_N_OP(72)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_I2C3_ER_USER_IRQ))
IRQ_VEC_N_OP(73)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_HS_EP1_OUT_USER_IRQ))
IRQ_VEC_N_OP(74)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_HS_EP1_IN_USER_IRQ))
IRQ_VEC_N_OP(75)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_HS_WKUP_USER_IRQ))
IRQ_VEC_N_OP(76)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_OTG_HS_USER_IRQ))
IRQ_VEC_N_OP(77)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_DCMI_USER_IRQ))
IRQ_VEC_N_OP(78)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_CRYP_USER_IRQ))
IRQ_VEC_N_OP(79)
#endif

#if !defined(USER_INTERRUPT) || \
	(defined(USER_INTERRUPT) && defined(CONFIG_HASH_RNG_USER_IRQ))
IRQ_VEC_N_OP(80)
#endif

#if !defined(USER_INTERRUPT) || (\
	defined(USER_INTERRUPT) && defined(CONFIG_FPU_USER_IRQ))
IRQ_VEC_N_OP(81)
#endif
