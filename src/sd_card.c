#include "sd_card.h"
#include "config.h"
#include "hw_config.h"  // ← agregar esto#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>
#include <stdio.h>

// ─── FatFS objeto global ──────────────────────
static FATFS fs;

// ─── Inicialización SPI1 ──────────────────────

sd_result_t sd_init(void) {
    // La inicialización SPI la maneja FatFS via hw_config.c
    // Solo montamos el filesystem
    if (!sd_init_driver()) {
        printf("[SD] Error inicializando driver\n");
        return SD_ERR_MOUNT;
    }

    FRESULT res = f_mount(&fs, "0:", 1);
    if (res != FR_OK) {
        printf("[SD] Error al montar: %d\n", res);
        return SD_ERR_MOUNT;
    }

    printf("[SD] Montada correctamente\n");
    return SD_OK;
}

// ─── Escaneo de fotos ─────────────────────────

int sd_scan_photos(char paths[][32], int max_count) {
    DIR dir;
    FILINFO fno;
    int count = 0;

    FRESULT res = f_opendir(&dir, PHOTOS_DIR);
    if (res != FR_OK) {
        printf("[SD] No se pudo abrir %s: %d\n", PHOTOS_DIR, res);
        return 0;
    }

    while (count < max_count) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == '\0') break;  // Fin del directorio
        if (fno.fattrib & AM_DIR) continue;               // Ignorar subdirectorios

        // Solo archivos .bmp (case-insensitive)
        char *ext = strrchr(fno.fname, '.');
        if (!ext || (strcasecmp(ext, ".bmp") != 0)) continue;

        snprintf(paths[count], 32, "%s/%s", PHOTOS_DIR, fno.fname);
        printf("[SD] Foto encontrada: %s\n", paths[count]);
        count++;
    }

    f_closedir(&dir);
    printf("[SD] Total fotos: %d\n", count);
    return count;
}

// ─── Parser BMP 24-bit ────────────────────────
// Formato BMP esperado: 24 bits por pixel, 240x240, sin compresión

// Convierte RGB888 a RGB565
static inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

sd_result_t sd_read_bmp(const char *path, uint16_t *out_buf) {
    FIL file;
    UINT br;
    uint8_t header[54];

    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        printf("[BMP] No se pudo abrir %s: %d\n", path, res);
        return SD_ERR_OPEN;
    }

    // Leer header BMP (54 bytes)
    res = f_read(&file, header, 54, &br);
    if (res != FR_OK || br != 54) {
        f_close(&file);
        return SD_ERR_READ;
    }

    // Validar firma BMP
    if (header[0] != 'B' || header[1] != 'M') {
        printf("[BMP] No es un archivo BMP valido\n");
        f_close(&file);
        return SD_ERR_FORMAT;
    }

    // Extraer metadatos del header
    uint32_t data_offset = *(uint32_t*)&header[10];
    int32_t  width       = *(int32_t*)&header[18];
    int32_t  height      = *(int32_t*)&header[22];
    uint16_t bpp         = *(uint16_t*)&header[28];
    uint32_t compression = *(uint32_t*)&header[30];

    printf("[BMP] %s: %dx%d, %d bpp\n", path, width, height, bpp);

    // Validar dimensiones y formato
    if (width != LCD_WIDTH || (height != LCD_HEIGHT && height != -LCD_HEIGHT)) {
        printf("[BMP] Dimensiones incorrectas: %dx%d (esperado %dx%d)\n",
               width, height, LCD_WIDTH, LCD_HEIGHT);
        f_close(&file);
        return SD_ERR_SIZE;
    }

    if (bpp != 24 || compression != 0) {
        printf("[BMP] Formato no soportado: %d bpp, compresion %d\n", bpp, compression);
        f_close(&file);
        return SD_ERR_FORMAT;
    }

    // BMP almacena filas de abajo hacia arriba
    // Cada fila tiene padding a múltiplo de 4 bytes
    bool flipped = (height > 0);  // height positivo = bottom-up
    int  rows    = (height < 0) ? -height : height;
    int  row_size_bytes = ((width * 3 + 3) / 4) * 4;  // con padding

    uint8_t row_buf[row_size_bytes];

    for (int row = 0; row < rows; row++) {
        // Calcular qué fila del BMP leer
        int bmp_row = flipped ? (rows - 1 - row) : row;
        FSIZE_t offset = data_offset + (FSIZE_t)bmp_row * row_size_bytes;

        f_lseek(&file, offset);
        res = f_read(&file, row_buf, row_size_bytes, &br);
        if (res != FR_OK || (int)br < width * 3) {
            f_close(&file);
            return SD_ERR_READ;
        }

        // Convertir BGR888 → RGB565 (BMP guarda en BGR)
        for (int col = 0; col < width; col++) {
            uint8_t b = row_buf[col * 3 + 0];
            uint8_t g = row_buf[col * 3 + 1];
            uint8_t r = row_buf[col * 3 + 2];
            out_buf[row * width + col] = rgb888_to_rgb565(r, g, b);
        }
    }

    f_close(&file);
    return SD_OK;
}
