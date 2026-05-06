
#ifndef ADC_H_
#define ADC_H_

void adc_continuous_conv_init(void);
uint32_t pa0_adc_read(void);
void adc_init_start(void);
void adc_pa0_interrupt_init(void);

#endif /* ADC_H_ */
