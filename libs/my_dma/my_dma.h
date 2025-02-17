#ifndef __MY_DMA_H__
#define __MY_DMA_H__

void my_dma_init(int pin_adc_init[], int size_pin_adc_init);
void my_dma_get_value(int pin_adc, int samples_length, int *ret);


#endif