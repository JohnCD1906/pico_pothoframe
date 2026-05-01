#include "st7789.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <string.h>

// ─── Comandos ST7789 ──────────────────────────
#define ST7789_NOP       0x00
#define ST7789_SWRESET   0x01
#define ST7789_SLPOUT    0x11
#define ST7789_NORON     0x13
#define ST7789_INVON     0x21
#define ST7789_DISPON    0x29
#define ST7789_CASET     0x2A
#define ST7789_RASET     0x2B
#define ST7789_RAMWR     0x2C
#define ST7789_MADCTL    0x36
#define ST7789_COLMOD    0x3A
#define ST7789_PORCTRL   0xB2
#define ST7789_GCTRL     0xB7
#define ST7789_VCOMS     0xBB
#define ST7789_LCMCTRL   0xC0
#define ST7789_VDVVRHEN  0xC2
#define ST7789_VRHS      0xC3
#define ST7789_VDVS      0xC4
#define ST7789_FRCTRL2   0xC6
#define ST7789_PWCTRL1   0xD0
#define ST7789_PVGAMCTRL 0xE0
#define ST7789_NVGAMCTRL 0xE1

// ─── Helpers internos ─────────────────────────

static inline void cs_select(void)   { /* ST7789 no usa CS en este módulo */ }
static inline void cs_deselect(void) { /* ST7789 no usa CS en este módulo */ }

static inline void dc_command(void) {
    gpio_put(PIN_LCD_DC, 0);
}

static inline void dc_data(void) {
    gpio_put(PIN_LCD_DC, 1);
}

static void write_cmd(uint8_t cmd) {
    dc_command();
    spi_write_blocking(LCD_SPI, &cmd, 1);
}

static void write_data(const uint8_t *data, size_t len) {
    dc_data();
    spi_write_blocking(LCD_SPI, data, len);
}

static void write_data_byte(uint8_t byte) {
    write_data(&byte, 1);
}

// ─── Secuencia de inicialización ──────────────

void st7789_init(void) {
    // Inicializar SPI0
    spi_init(LCD_SPI, LCD_CLK_HZ);
    spi_set_format(LCD_SPI, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    gpio_set_function(PIN_LCD_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_LCD_RES); gpio_set_dir(PIN_LCD_RES, GPIO_OUT); gpio_put(PIN_LCD_RES, 1);
    gpio_init(PIN_LCD_DC);  gpio_set_dir(PIN_LCD_DC,  GPIO_OUT); gpio_put(PIN_LCD_DC,  1);
    gpio_init(PIN_LCD_BLK); gpio_set_dir(PIN_LCD_BLK, GPIO_OUT); gpio_put(PIN_LCD_BLK, 0);

    // Reset hardware
    gpio_put(PIN_LCD_RES, 1); sleep_ms(10);
    gpio_put(PIN_LCD_RES, 0); sleep_ms(10);
    gpio_put(PIN_LCD_RES, 1); sleep_ms(120);

    // Secuencia mínima probada
    write_cmd(0x01); sleep_ms(100);          // Software reset
    write_cmd(0x11); sleep_ms(50);           // Sleep out
    write_cmd(0x3A); write_data_byte(0x55);  // Color mode 16bit RGB565
    sleep_ms(10);
    write_cmd(0x36); write_data_byte(0x00);  // MADCTL
    write_cmd(0x21); sleep_ms(10);           // Inversion ON
    write_cmd(0x13); sleep_ms(10);           // Normal display ON
    write_cmd(0x29); sleep_ms(50);           // Display ON

    // Pantalla en negro y backlight ON
    st7789_fill(COLOR_BLACK);
    gpio_put(PIN_LCD_BLK, 1);
}

// ─── Ventana de escritura ─────────────────────

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t caset[] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    uint8_t raset[] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};

    write_cmd(ST7789_CASET); write_data(caset, 4);
    write_cmd(ST7789_RASET); write_data(raset, 4);
    write_cmd(ST7789_RAMWR);
}

// ─── Escritura de píxeles ─────────────────────

void st7789_write_pixels(const uint16_t *buf, uint32_t len) {
    dc_data();
    // Enviar en big-endian (ST7789 espera MSB primero)
    for (uint32_t i = 0; i < len; i++) {
        uint8_t hi = buf[i] >> 8;
        uint8_t lo = buf[i] & 0xFF;
        spi_write_blocking(LCD_SPI, &hi, 1);
        spi_write_blocking(LCD_SPI, &lo, 1);
    }
}

// ─── Funciones de alto nivel ──────────────────

void st7789_fill(uint16_t color) {
    st7789_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    dc_data();
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        spi_write_blocking(LCD_SPI, &hi, 1);
        spi_write_blocking(LCD_SPI, &lo, 1);
    }
}

void st7789_draw_hline(uint16_t y, uint16_t x0, uint16_t x1, const uint16_t *pixels) {
    st7789_set_window(x0, y, x1, y);
    st7789_write_pixels(pixels, x1 - x0 + 1);
}

void st7789_backlight(bool on) {
    gpio_put(PIN_LCD_BLK, on ? 1 : 0);
}
