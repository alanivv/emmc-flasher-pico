#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define PIN_CLK   10
#define PIN_CMD   11
#define PIN_DAT0  12

PIO pio = pio0;
uint sm_read = 0;
uint offset_read = 0;

static uint8_t block_buffer[512];

// Bytecode directo de la máquina PIO (no requiere pioasm externo)
static const uint16_t emmc_read_instructions[] = {
    0x4101, // in pins, 1  side 0 [1]
    0xb142  // nop         side 1 [1]
};

static const struct pio_program emmc_read_program = {
    .instructions = emmc_read_instructions,
    .length = 2,
    .origin = -1,
};

void set_pio_frequency(float target_freq_hz) {
    pio_sm_set_enabled(pio, sm_read, false);
    float sys_clk = (float)clock_get_hz(clk_sys);
    float div = sys_clk / (target_freq_hz * 4.0f);
    pio_sm_set_clkdiv(pio, sm_read, div);
    pio_sm_set_enabled(pio, sm_read, true);
}

void init_hardware() {
    stdio_init_all();
    
    gpio_init(PIN_CLK);
    gpio_init(PIN_CMD);
    gpio_init(PIN_DAT0);
    
    gpio_set_dir(PIN_CLK, GPIO_OUT);
    gpio_set_dir(PIN_CMD, GPIO_OUT);
    gpio_set_dir(PIN_DAT0, GPIO_IN);
    
    gpio_pull_up(PIN_CMD);
    gpio_pull_up(PIN_DAT0);

    offset_read = pio_add_program(pio, &emmc_read_program);
    pio_sm_config c = pio_get_default_sm_config();
    
    sm_config_set_wrap(&c, offset_read + 0, offset_read + 1);
    sm_config_set_sideset(&c, 1, false, false);
    sm_config_set_sideset_pins(&c, PIN_CLK);
    sm_config_set_in_pins(&c, PIN_DAT0);
    sm_config_set_in_shift(&c, false, true, 8);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    
    pio_gpio_init(pio, PIN_CLK);
    pio_gpio_init(pio, PIN_DAT0);
    pio_sm_init(pio, sm_read, offset_read, &c);
    
    set_pio_frequency(3000000.0f); // 3.0 MHz seguro
}

int main() {
    init_hardware();

    while (1) {
        int cmd = getchar_timeout_us(1000);
        if (cmd == 'F') { 
            int freq_mhz = getchar_timeout_us(10000);
            if (freq_mhz > 0 && freq_mhz <= 20) {
                set_pio_frequency((float)freq_mhz * 1000000.0f);
            }
        } else if (cmd == 'R') {
            for (int blk = 0; blk < 8192; blk++) {
                for (int i = 0; i < 512; i++) {
                    block_buffer[i] = pio_sm_get_blocking(pio, sm_read);
                }
                fwrite(block_buffer, 1, 512, stdout);
            }
            fflush(stdout);
        }
    }
    return 0;
}