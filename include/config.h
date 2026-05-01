#ifndef CONFIG_H
#define CONFIG_H

#include "pico/stdlib.h"
#include "hardware/spi.h"

// ─────────────────────────────────────────────
//  SPI0 → Pantalla ST7789
// ─────────────────────────────────────────────
#define LCD_SPI       spi0
#define LCD_CLK_HZ    (40 * 1000 * 1000)   // 40 MHz

#define PIN_LCD_SCK   2   // GP2 → SPI0 SCK  (Pin 4)
#define PIN_LCD_MOSI  3   // GP3 → SPI0 TX   (Pin 5)  ← corregido
#define PIN_LCD_RES   4   // GP4              (Pin 6)
#define PIN_LCD_DC    5   // GP5              (Pin 7)
#define PIN_LCD_BLK   6   // GP6              (Pin 9)

// ─────────────────────────────────────────────
//  SPI1 → MicroSD
// ─────────────────────────────────────────────
#define SD_SPI        spi1
#define SD_CLK_HZ     (25 * 1000 * 1000)   // 25 MHz

#define PIN_SD_SCK    10
#define PIN_SD_MOSI   11
#define PIN_SD_MISO   12
#define PIN_SD_CS     13

// ─────────────────────────────────────────────
//  Pantalla
// ─────────────────────────────────────────────
#define LCD_WIDTH     240
#define LCD_HEIGHT    240

// ─────────────────────────────────────────────
//  Slideshow
// ─────────────────────────────────────────────
#define SLIDE_DELAY_MS       5000
#define TRANSITION_STEPS     30
#define TRANSITION_DELAY_MS  16

// ─────────────────────────────────────────────
//  Formato de imagen en SD
// ─────────────────────────────────────────────
#define PHOTOS_DIR    "/photos"
#define MAX_PHOTOS    64

#endif // CONFIG_H
