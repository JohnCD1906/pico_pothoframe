#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdint.h>
#include <stdbool.h>
#include "ff.h"   // FatFS

// ─── Resultado de operaciones ─────────────────
typedef enum {
    SD_OK = 0,
    SD_ERR_MOUNT,
    SD_ERR_OPEN,
    SD_ERR_FORMAT,      // No es BMP válido
    SD_ERR_SIZE,        // Dimensiones incorrectas
    SD_ERR_READ,
} sd_result_t;

// ─── API pública ──────────────────────────────

/**
 * Inicializa SPI1 y monta el sistema de archivos FAT.
 * Retorna SD_OK si todo está bien.
 */
sd_result_t sd_init(void);

/**
 * Escanea /photos/ y llena `paths` con los nombres de archivos .bmp encontrados.
 * `max_count` es el tamaño del arreglo paths.
 * Retorna el número de archivos encontrados.
 */
int sd_scan_photos(char paths[][32], int max_count);

/**
 * Lee un archivo BMP 24-bit 240x240 y lo convierte a RGB565.
 * `out_buf` debe tener espacio para 240*240 uint16_t (115200 bytes).
 * Retorna SD_OK en éxito.
 */
sd_result_t sd_read_bmp(const char *path, uint16_t *out_buf);

#endif // SD_CARD_H
