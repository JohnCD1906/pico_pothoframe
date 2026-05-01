#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <stdint.h>
#include "config.h"

// ─── Tipos de transición ──────────────────────
typedef enum {
    TRANSITION_FADE,        // Fade a negro y aparece la siguiente
    TRANSITION_SLIDE_LEFT,  // Desliza de derecha a izquierda
    TRANSITION_SLIDE_UP,    // Desliza de abajo hacia arriba
} transition_t;

// ─── API pública ──────────────────────────────

/**
 * Inicializa el módulo de slideshow.
 * Escanea la SD en busca de fotos y carga la primera.
 */
void slideshow_init(void);

/**
 * Loop principal del slideshow.
 * Llama esto desde main() — no retorna.
 */
void slideshow_run(void);

/**
 * Muestra una imagen en pantalla inmediatamente (sin transición).
 */
void slideshow_show_image(const uint16_t *img);

/**
 * Realiza una transición entre la imagen actual y la siguiente.
 */
void slideshow_transition(const uint16_t *from, const uint16_t *to, transition_t type);

#endif // SLIDESHOW_H
