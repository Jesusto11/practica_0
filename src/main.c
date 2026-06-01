/*
Integrantes
Aaren Asher Juárez Arellano 10442
Jesús Torres Vargas 10511

CONCLUSIÓN
¿Cuál fue el efecto de modificar las prioridades de las tareas?
R= Se modifica el orden en el que el microcontrolador ejecuta cada tarea, lo cual podemos ver en la consola por el orden en el que
se imprime cada segundo.

¿Que observaron en el consumo de stack de cada tarea y por qué difieren?
R= Difieren porque cada tarea ejecuta un conjunto distinto de instrucciones y variables locales. La que menos 

¿Como se refleja el comportamiento del planificador preemptivo en la salida de terminal?
R= Por los tiempos de ejecución de cada tarea, el orden en el que se imprimen es siempre el mismo a excepción de cuando se cumple cada 1000ms,
que es el mínimo común múltiplo de 200ms, 500ms, 1000ms. Esto se traduce en que a los 1000ms es cuando se cumplen los tiempos de las 3 tareas
pero se ejecutarán en el orden de prioridad más alta a más baja.


*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Definición de pines
#define LED_PIN 4
#define BUTTON_PIN 5

TaskHandle_t hT1, hT2, hT3;
int estado = 0; // Variable global para el estado del LED

/* -------- vTaskLED (prioridad 1, cada 500 ms) -------- */
void vTaskLED(void *pvParameters) {
    for(;;) {
        // Funciona como un GPIO_toggle
        estado = !estado; 
        gpio_set_level(LED_PIN, estado);
        
        printf("[LED]    -> %s \ttick: %lu\r\n", estado ? "ON" : "OFF", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* -------- vTaskMonitor (prioridad 2, cada 1000 ms) ---- */
void vTaskMonitor(void *pvParameters) {
    for(;;) {
        printf("[MON] Heap libre: %u bytes\r\n", (unsigned)xPortGetFreeHeapSize());
        printf("[MON] Stack T1 min: %u T2: %u T3: %u words\r\n",
               uxTaskGetStackHighWaterMark(hT1),
               uxTaskGetStackHighWaterMark(hT2),
               uxTaskGetStackHighWaterMark(hT3));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* -------- vTaskSensor (prioridad 3, cada 200 ms) ------ */
void vTaskSensor(void *pvParameters) {
    for(;;) {
        // Equivalente a leer_adc_o_boton() del PDF
        uint16_t val = gpio_get_level(BUTTON_PIN);
        
        printf("[SENS] val: %4u \ttick: %lu\r\n", val, xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    xTaskCreate(vTaskLED, "TaskLED", 2048, NULL, 1, &hT1);
    xTaskCreate(vTaskMonitor, "TaskMonitor", 2048, NULL, 2, &hT2);
    xTaskCreate(vTaskSensor, "TaskSensor", 2048, NULL, 3, &hT3);
    printf("-----------------");
}

