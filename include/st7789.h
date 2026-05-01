#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// ─── Colores RGB565 útiles ───────────────────
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F

// ─── API pública ─────────────────────────────

/**
 * Inicializa SPI0 y la pantalla ST7789.
 * Debe llamarse una sola vez al inicio.
 */
void st7789_init(void);

/**
 * Llena toda la pantalla con un color RGB565.
 */
void st7789_fill(uint16_t color);

/**
 * Define la ventana de escritura (región activa).
 */
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * Envía un buffer de píxeles RGB565 a la ventana activa.
 * len = número de píxeles (no bytes).
 */
void st7789_write_pixels(const uint16_t *buf, uint32_t len);

/**
 * Dibuja una línea horizontal de píxeles RGB565
 * en la fila `y`, desde `x0` hasta `x1`.
 */
void st7789_draw_hline(uint16_t y, uint16_t x0, uint16_t x1, const uint16_t *pixels);

/**
 * Enciende o apaga el backlight.
 */
void st7789_backlight(bool on);

#endif // ST7789_H
