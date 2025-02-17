#include "hardware/adc.h"
#include "hardware/dma.h"
#include "my_dma.h"
#include "my_std_debug.h"

// Número de maximo de canais ADC
#define ADC_CHANNELS_MAX 2UL

// offset para relacionar o pino com seu respectivo canal
#define ADC_OFFSET_PIN_TO_CHANNEL 26UL

// tamanho da regiao de memoria manipulada pelo DMA
#define BUFFER_SIZE 256UL


static void adc_dma_handler();



// memoria manipulada pelo DMA
static uint16_t adc_buffer[BUFFER_SIZE];


// Canal DMA
static int dma_chan;

void my_dma_init(int pin_adc_init[], int size_pin_adc_init)
{
    // verifica se o tamanho do array eh valido
    if(size_pin_adc_init > ADC_CHANNELS_MAX)
    {
        MY_STD_DEBUG_MSG("size_pin_adc_init invalid, this is more large");
    }

    // Inicializa o ADC e configura o canal
    adc_init();

    for(register int i = 0; i < size_pin_adc_init; i++)
        adc_gpio_init(pin_adc_init[i]);

    
    int value_write_reg = 0;
    
    // prepara os canais para o dma ficar alternando
    for(register int i = 0; i < size_pin_adc_init; i++)
        value_write_reg |= ( 1 << (pin_adc_init[i]-ADC_OFFSET_PIN_TO_CHANNEL) );
    
    // Configura o modo round-robin para alternar entre os canais passados por parametro
    adc_set_round_robin(value_write_reg);

      // Configura FIFO do ADC
    adc_fifo_setup(
        true,  // Ativa FIFO
        true,  // Aciona DMA quando há dados
        1,     // Solicitação de DMA a cada amostra
        false, // Sem multiplicação de valores
        false  // FIFO em 16 bits (incluindo os 4 bits de canal) OBS: nao consegui obter o canal em funcao desses 4 bits msb
    );

    // Inicia conversão contínua
    adc_run(true);

    // Configuração do DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);

    // Configura DMA para transferir dados do ADC para o buffer
    dma_channel_configure(
        dma_chan, &c,
        adc_buffer,    // Destino: buffer
        &adc_hw->fifo, // Fonte: FIFO do ADC
        BUFFER_SIZE,   // Tamanho da transferência
        false          // Não inicia ainda
    );

    // Configura interrupção do DMA
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, &adc_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Inicia a primeira transferência
    dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}


void my_dma_get_value(int pin_adc, int samples_length, int *ret)
{
    if(ret == NULL)
    {
        return;
    }

    // determina os incrementos necessaros para obter apenas os valores do canal passado como parametro
    int next_step = (pin_adc-ADC_OFFSET_PIN_TO_CHANNEL) % ADC_CHANNELS_MAX;

    for(register int i = 0, index = next_step; i < samples_length; i++, index += 2)
    {
        // extrai os 12 bits
        ret[i] = adc_buffer[index] & 0xFFF;
    }
}

static void adc_dma_handler()
{
  // Limpa a flag da interrupção do DMA
  dma_hw->ints0 = 1u << dma_chan;

  // Reinicia a transferência do DMA
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}