#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

static inline void spi_reset(spi_inst_t *spi) {
    invalid_params_if(HARDWARE_SPI, spi != spi0 && spi != spi1);
    reset_block_num(spi == spi0 ? RESET_SPI0 : RESET_SPI1);
}

static inline void spi_unreset(spi_inst_t *spi) {
    invalid_params_if(HARDWARE_SPI, spi != spi0 && spi != spi1);
    unreset_block_num_wait_blocking(spi == spi0 ? RESET_SPI0 : RESET_SPI1);
}

uint spi_init_with_bits(spi_inst_t *spi, uint baudrate, uint bits) {
    spi_reset(spi);
    spi_unreset(spi);

    uint baud = spi_set_baudrate(spi, baudrate);
    spi_set_format(spi, bits, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    // Always enable DREQ signals -- harmless if DMA is not listening
    hw_set_bits(&spi_get_hw(spi)->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);

    // Finally enable the SPI
    hw_set_bits(&spi_get_hw(spi)->cr1, SPI_SSPCR1_SSE_BITS);

    return baud;
}

uint mcp3208_get_adc_single(uint ch)
{
    uint result = 0;
    uint16_t txbuf[2];
    uint16_t rxbuf[2];

    txbuf[0] = ((ch & 7) << 4) | 0x180; // (1),Start(1)=1,SGL/DIF(1)=1,ch(3)=0~7,(4)
    txbuf[1] = 0;
    cs_select();
    spi_write16_read16_blocking(SPI_PORT, txbuf, rxbuf, 2);
    cs_deselect();
    result = (rxbuf[0] << 10) + rxbuf[1];

    return result & 0xfff;
}

int main()
{
    stdio_init_all();

    printf("Hello, MCP3208! Geting data via SPI...\n");

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init_with_bits(SPI_PORT, 1000*1000, 10);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    printf("SPI baudrate %dHz\n", spi_get_baudrate(SPI_PORT));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    int ch = 0;
    while (true) {
        uint16_t data = mcp3208_get_adc_single(ch);

        printf("ch=%d, value=%4d\n", ch, data);
        ch = ch == 7 ? 0 : ch + 1;

        sleep_ms(1000);
    }
}
