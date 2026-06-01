#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define el pin donde conectarás el LED físico. 
// Usaremos el GPIO 4 como ejemplo.
#define BLINK_GPIO 4

void app_main(void) {
    // 1. Configurar el pin del LED como salida
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;

    // 2. Mensaje de prueba inicial por UART (Actividad 1)
    printf("¡Sistema iniciado! Probando comunicacion UART y LED...\n");

    // 3. Bucle infinito (comportamiento base de una tarea en FreeRTOS)
    while (1) {
        // Alternar el estado del LED
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, led_state);
        
        // Imprimir el estado actual por el monitor serie
        printf("Estado del LED: %s\n", led_state ? "ON" : "OFF");
        
        // Bloqueo temporal: cede el procesador por 1000 ms usando la API de FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}