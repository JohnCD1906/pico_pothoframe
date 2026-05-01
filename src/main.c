#include "pico/stdlib.h"
#include "st7789.h"
#include "sd_card.h"
#include "slideshow.h"
#include <stdio.h>

int main(void) {
    // Inicializar stdio (UART para debug)
    stdio_init_all();
    sleep_ms(2000);  // Dar tiempo al terminal de conectarse

    printf("\n=============================\n");
    printf("  Pico Photo Frame v1.0\n");
    printf("=============================\n\n");

    // 1. Inicializar pantalla
    printf("[Init] Iniciando pantalla ST7789...\n");
    st7789_init();
    printf("[Init] Pantalla OK\n");

    // 2. Inicializar SD
    printf("[Init] Montando microSD...\n");
    sd_result_t sd_res = sd_init();
    if (sd_res != SD_OK) {
        printf("[Init] ERROR: No se pudo montar la SD (%d)\n", sd_res);
        // Parpadear rojo en pantalla para indicar error
        while (true) {
            st7789_fill(COLOR_RED);
            sleep_ms(500);
            st7789_fill(COLOR_BLACK);
            sleep_ms(500);
        }
    }
    printf("[Init] SD OK\n");

    // 3. Iniciar slideshow (escanea fotos y muestra la primera)
    printf("[Init] Iniciando slideshow...\n");
    slideshow_init();

    // 4. Loop infinito del slideshow
    printf("[Init] Entrando al loop principal\n\n");
    slideshow_run();

    // No debería llegar aquí
    return 0;
/*    stdio_init_all();
    st7789_init();
    st7789_fill(COLOR_RED);  // Debe verse rojo
    while(true) tight_loop_contents();*/
}
