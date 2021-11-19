//See LICENSE for license details.

#include "config.h"
#include "thread.h"

extern uint32_t disable_mcycle_minstret();
void _init() {
	SystemInit();
	eclic_init(ECLIC_NUM_INTERRUPTS);
	eclic_mode_enable();
	disable_mcycle_minstret();
}

void _fini() {}



void RCUInit() {
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_USART0);
	rcu_periph_clock_enable(RCU_TIMER6);
	rcu_periph_clock_enable(RCU_AF);
}



//pa9:uart_tx
//pa10:uart_rx
void GPIOInit() {
	///pa0
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);



	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}

//timer6:1ms触发一次
void TimerInit() {
	timer_parameter_struct timer_initpara;

	timer_deinit(TIMER6);
	timer_struct_para_init(&timer_initpara);
	timer_initpara.prescaler = 10799;	//108M/10800 = 10K Hz
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = (uint32_t)9 * 1;// 0.9ms
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_init(TIMER6, &timer_initpara);

	timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER6, TIMER_INT_UP);	//update interrupt
}



//usart1:115200
void USARTInit() {
	usart_deinit(USART0);
	usart_baudrate_set(USART0, 115200U);
	usart_word_length_set(USART0, USART_WL_8BIT);
	usart_stop_bit_set(USART0, USART_STB_1BIT);
	usart_parity_config(USART0, USART_PM_NONE);
	usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);


	while (RESET == usart_flag_get(USART0, USART_FLAG_TC));
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	usart_enable(USART0);
}



void InterruptInit() {
	eclic_global_interrupt_enable();
	eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);

	eclic_irq_enable(USART0_IRQn, 1, 0);
	eclic_irq_enable(TIMER6_IRQn, 2, 0);


}





void Init() {
	_init();

	RCUInit();
	InterruptInit();
	GPIOInit();
	USARTInit();
	TimerInit();

	Lcd_Init();
	LCD_Clear(WHITE);

}




int _put_char(int ch) {

	thd_stop;
	usart_data_transmit(USART0, (uint8_t)ch);
	while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
	}
	thd_cont;

	return ch;
}
