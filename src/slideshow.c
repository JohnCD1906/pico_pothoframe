#include "slideshow.h"
#include "st7789.h"
#include "sd_card.h"
#include "config.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ─── Estado interno ───────────────────────────

// Dos buffers: uno para imagen actual, otro para la siguiente
// Cada buffer: 240*240*2 bytes = 115200 bytes (~112 KB)
static uint16_t buf_a[LCD_WIDTH * LCD_HEIGHT];
static uint16_t buf_b[LCD_WIDTH * LCD_HEIGHT];

static char photo_paths[MAX_PHOTOS][32];
static int  photo_count  = 0;
static int  photo_index  = 0;

// Punteros a buffer actual y siguiente
static uint16_t *cur_buf  = buf_a;
static uint16_t *next_buf = buf_b;

// ─── Mostrar imagen completa ──────────────────

void slideshow_show_image(const uint16_t *img) {
    st7789_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    st7789_write_pixels(img, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}

// ─── Transición: Fade a negro ─────────────────

static void transition_fade(const uint16_t *from, const uint16_t *to) {
    static uint16_t tmp[LCD_WIDTH];

    // Fase 1: Oscurecer imagen actual
    for (int step = TRANSITION_STEPS; step >= 0; step--) {
        for (int row = 0; row < LCD_HEIGHT; row++) {
            for (int col = 0; col < LCD_WIDTH; col++) {
                uint16_t px = from[row * LCD_WIDTH + col];
                // Escalar cada canal por (step / TRANSITION_STEPS)
                uint8_t r = ((px >> 11) & 0x1F) * step / TRANSITION_STEPS;
                uint8_t g = ((px >> 5)  & 0x3F) * step / TRANSITION_STEPS;
                uint8_t b = ( px        & 0x1F) * step / TRANSITION_STEPS;
                tmp[col] = (r << 11) | (g << 5) | b;
            }
            st7789_draw_hline(row, 0, LCD_WIDTH - 1, tmp);
        }
        sleep_ms(TRANSITION_DELAY_MS);
    }

    // Fase 2: Aclarar hacia la nueva imagen
    for (int step = 0; step <= TRANSITION_STEPS; step++) {
        for (int row = 0; row < LCD_HEIGHT; row++) {
            for (int col = 0; col < LCD_WIDTH; col++) {
                uint16_t px = to[row * LCD_WIDTH + col];
                uint8_t r = ((px >> 11) & 0x1F) * step / TRANSITION_STEPS;
                uint8_t g = ((px >> 5)  & 0x3F) * step / TRANSITION_STEPS;
                uint8_t b = ( px        & 0x1F) * step / TRANSITION_STEPS;
                tmp[col] = (r << 11) | (g << 5) | b;
            }
            st7789_draw_hline(row, 0, LCD_WIDTH - 1, tmp);
        }
        sleep_ms(TRANSITION_DELAY_MS);
    }
}

// ─── Transición: Slide hacia la izquierda ─────

static void transition_slide_left(const uint16_t *from, const uint16_t *to) {
    static uint16_t tmp[LCD_WIDTH];

    for (int step = 0; step <= TRANSITION_STEPS; step++) {
        int offset = (LCD_WIDTH * step) / TRANSITION_STEPS;

        for (int row = 0; row < LCD_HEIGHT; row++) {
            // Parte izquierda: imagen nueva
            for (int col = 0; col < offset; col++) {
                tmp[col] = to[row * LCD_WIDTH + (LCD_WIDTH - offset + col)];
            }
            // Parte derecha: imagen actual desplazada
            for (int col = offset; col < LCD_WIDTH; col++) {
                tmp[col] = from[row * LCD_WIDTH + col - offset];
            }
            st7789_draw_hline(row, 0, LCD_WIDTH - 1, tmp);
        }
        sleep_ms(TRANSITION_DELAY_MS);
    }
}

// ─── Transición: Slide hacia arriba ──────────

static void transition_slide_up(const uint16_t *from, const uint16_t *to) {
    for (int step = 0; step <= TRANSITION_STEPS; step++) {
        int offset = (LCD_HEIGHT * step) / TRANSITION_STEPS;

        // Filas que muestran la imagen nueva (parte superior)
        for (int row = 0; row < offset; row++) {
            int src_row = LCD_HEIGHT - offset + row;
            st7789_draw_hline(row, 0, LCD_WIDTH - 1,
                              &to[src_row * LCD_WIDTH]);
        }
        // Filas que muestran la imagen actual (parte inferior)
        for (int row = offset; row < LCD_HEIGHT; row++) {
            st7789_draw_hline(row, 0, LCD_WIDTH - 1,
                              &from[(row - offset) * LCD_WIDTH]);
        }
        sleep_ms(TRANSITION_DELAY_MS);
    }
}

// ─── Dispatcher de transiciones ──────────────

void slideshow_transition(const uint16_t *from, const uint16_t *to, transition_t type) {
    switch (type) {
        case TRANSITION_FADE:       transition_fade(from, to);       break;
        case TRANSITION_SLIDE_LEFT: transition_slide_left(from, to); break;
        case TRANSITION_SLIDE_UP:   transition_slide_up(from, to);   break;
        default:                    slideshow_show_image(to);        break;
    }
}

// ─── Inicialización ───────────────────────────

void slideshow_init(void) {
    photo_count = sd_scan_photos(photo_paths, MAX_PHOTOS);

    if (photo_count == 0) {
        printf("[Slideshow] No se encontraron fotos en %s\n", PHOTOS_DIR);
        // Mostrar pantalla azul de "sin fotos"
        st7789_fill(COLOR_BLUE);
        return;
    }

    // Cargar la primera foto
    printf("[Slideshow] Cargando primera foto: %s\n", photo_paths[0]);
    sd_result_t r = sd_read_bmp(photo_paths[0], cur_buf);
    if (r == SD_OK) {
        slideshow_show_image(cur_buf);
    } else {
        printf("[Slideshow] Error cargando foto: %d\n", r);
        st7789_fill(COLOR_RED);
    }
}

// ─── Loop principal ───────────────────────────

void slideshow_run(void) {
    if (photo_count == 0) return;

    // Ciclar transiciones para variar
    transition_t transitions[] = {
        TRANSITION_FADE,
        TRANSITION_SLIDE_LEFT,
        TRANSITION_SLIDE_UP,
    };
    int num_transitions = sizeof(transitions) / sizeof(transitions[0]);

    while (true) {
        // Esperar el tiempo configurado
        sleep_ms(SLIDE_DELAY_MS);

        // Avanzar al siguiente índice (circular)
        photo_index = (photo_index + 1) % photo_count;
        printf("[Slideshow] Pasando a foto %d: %s\n",
               photo_index, photo_paths[photo_index]);

        // Cargar siguiente foto en el buffer libre
        sd_result_t r = sd_read_bmp(photo_paths[photo_index], next_buf);
        if (r != SD_OK) {
            printf("[Slideshow] Error leyendo foto, saltando...\n");
            continue;
        }

        // Elegir transición basada en el índice
        transition_t tr = transitions[photo_index % num_transitions];
        slideshow_transition(cur_buf, next_buf, tr);

        // Intercambiar buffers
        uint16_t *tmp = cur_buf;
        cur_buf  = next_buf;
        next_buf = tmp;
    }
}
