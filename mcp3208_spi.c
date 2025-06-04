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

typedef enum {
    SPI_Motorola = 0,
    SPI_TI = 1,
    SPI_National = 2,
} spi_frame_format_t;

static inline void spi_reset(spi_inst_t *spi) {
    invalid_params_if(HARDWARE_SPI, spi != spi0 && spi != spi1);
    reset_block_num(spi == spi0 ? RESET_SPI0 : RESET_SPI1);
}

static inline void spi_unreset(spi_inst_t *spi) {
    invalid_params_if(HARDWARE_SPI, spi != spi0 && spi != spi1);
    unreset_block_num_wait_blocking(spi == spi0 ? RESET_SPI0 : RESET_SPI1);
}

static inline void spi_set_format_frf(spi_inst_t *spi, uint data_bits, spi_cpol_t cpol, spi_cpha_t cpha, __unused spi_order_t order, spi_frame_format_t frf) {
    invalid_params_if(HARDWARE_SPI, data_bits < 4 || data_bits > 16);
    // LSB-first not supported on PL022:
    invalid_params_if(HARDWARE_SPI, order != SPI_MSB_FIRST);
    invalid_params_if(HARDWARE_SPI, cpol != SPI_CPOL_0 && cpol != SPI_CPOL_1);
    invalid_params_if(HARDWARE_SPI, cpha != SPI_CPHA_0 && cpha != SPI_CPHA_1);
    invalid_params_if(HARDWARE_SPI, frf != SPI_Motorola && frf != SPI_TI && frf != SPI_National);

    // Disable the SPI
    uint32_t enable_mask = spi_get_hw(spi)->cr1 & SPI_SSPCR1_SSE_BITS;
    hw_clear_bits(&spi_get_hw(spi)->cr1, SPI_SSPCR1_SSE_BITS);

    hw_write_masked(&spi_get_hw(spi)->cr0,
                    ((uint)(data_bits - 1)) << SPI_SSPCR0_DSS_LSB |
                    ((uint)frf) << SPI_SSPCR0_FRF_LSB |
                    ((uint)cpol) << SPI_SSPCR0_SPO_LSB |
                    ((uint)cpha) << SPI_SSPCR0_SPH_LSB,
        SPI_SSPCR0_DSS_BITS |
        SPI_SSPCR0_FRF_BITS |
        SPI_SSPCR0_SPO_BITS |
        SPI_SSPCR0_SPH_BITS);

    // Re-enable the SPI
    hw_set_bits(&spi_get_hw(spi)->cr1, enable_mask);
}

uint spi_init_with_bits_and_frf(spi_inst_t *spi, uint baudrate, uint bits, uint frf) {
    spi_reset(spi);
    spi_unreset(spi);

    uint baud = spi_set_baudrate(spi, baudrate);
    spi_set_format_frf(spi, bits, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST, frf);
    // Always enable DREQ signals -- harmless if DMA is not listening
    hw_set_bits(&spi_get_hw(spi)->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);

    // Finally enable the SPI
    hw_set_bits(&spi_get_hw(spi)->cr1, SPI_SSPCR1_SSE_BITS);

    return baud;
}

uint mcp3208_get_adc_single(uint ch)
{
    uint result = 0;
    uint16_t txbuf[1];
    uint16_t rxbuf[1];

    txbuf[0] = ((ch & 7) << 1) | 0x30; // (2),Start(1)=1,SGL/DIF(1)=1,ch(3)=0~7,(1)
    spi_write16_read16_blocking(SPI_PORT, txbuf, rxbuf, 1);
    result = rxbuf[0];

    return result & 0xfff;
}

int main()
{
    stdio_init_all();

    printf("Hello, MCP3208! Geting data via SPI...\n");

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init_with_bits_and_frf(SPI_PORT, 1000*1000, 12, SPI_National);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    printf("SPI baudrate %dHz\n", spi_get_baudrate(SPI_PORT));

    int ch = 0;
    while (true) {
        uint16_t data = mcp3208_get_adc_single(ch);

        printf("ch=%d, value=%4d\n", ch, data);
        ch = ch == 7 ? 0 : ch + 1;

        sleep_ms(1000);
    }
}
