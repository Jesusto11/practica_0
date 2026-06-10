#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define Led 2
#define bot 4

// Configuración del ADC
#define ADC_RESOLUTION_BITS 12U
#define ADC_MAX_VALUE       ((1U << ADC_RESOLUTION_BITS) - 1U)
#define ADC_CH_Sensor       ADC_CHANNEL_6

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static const char *TAG = "ADC_READER";

uint16_t adc;
float voltaje = 0.0f;

TaskHandle_t Task_1 = NULL;
TaskHandle_t Task_2 = NULL;
TaskHandle_t Task_3 = NULL;
TimerHandle_t TiempoBloqueo = NULL;

void vTimerCallback(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "Tiempo de bloqueo expirado, reanudando Tarea 1 (Rapido)");
    vTaskResume(Task_1);
}

uint16_t adc_reader_get_raw(adc_channel_t channel)
{
    if (s_adc_handle == NULL) {
        return 0U;
    }
    int raw = 0;
    if (adc_oneshot_read(s_adc_handle, channel, &raw) == ESP_OK) {
        return (uint16_t)raw;
    }
    return 0U;
}

void adc_reader_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &s_adc_handle));
    
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,     
        .bitwidth = ADC_BITWIDTH_DEFAULT,  
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_CH_Sensor, &chan_cfg));

    ESP_LOGI(TAG, "ADC Inicializado con éxito");
}

void vTaskLedRapido(void *pvParameters)
{
    // Suspensión inicial: espera la orden del Timer o del Sensor
    vTaskSuspend(NULL);
    
    while(1)
    {
        int Tiempo = 0;
        while (Tiempo < 25) {
            gpio_set_level(Led, 1);
            vTaskDelay(pdMS_TO_TICKS(100));

            gpio_set_level(Led, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            Tiempo++;
        }
        vTaskResume(Task_2); // Llama al Led Lento
        vTaskSuspend(NULL);  // Se duerme hasta la próxima ejecución
    }
}

void vTaskLedLento(void *pvParameters)
{
    // Suspensión inicial: espera la orden de la Tarea 1
    vTaskSuspend(NULL);
    
    while(1)
    {
        int Tiempo = 0;
        while (Tiempo < 5) {
            gpio_set_level(Led, 1);
            vTaskDelay(pdMS_TO_TICKS(500));

            gpio_set_level(Led, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            Tiempo++;
        }
        
        // Al terminar el ciclo completo, reactivamos el timer de 5s
        xTimerStart(TiempoBloqueo, 0);
        vTaskSuspend(NULL); // Se duerme
    }
}

void vTaskSensor(void *pvParameters)
{
    // Suspensión inicial: espera la orden del botón
    vTaskSuspend(NULL);
    
    while(1)
    {
        int Tiempo = 0;
        while(Tiempo < 1)
        {
            adc = adc_reader_get_raw(ADC_CH_Sensor);
            voltaje = (float)adc * 3.3f / (float)ADC_MAX_VALUE;
            ESP_LOGI("Sensor", "Valor ADC: %u, Voltaje: %.2f V", adc, voltaje);
            vTaskDelay(pdMS_TO_TICKS(300));
            Tiempo++;
        }
        vTaskResume(Task_1); // Llama al Led Rapido
        vTaskSuspend(NULL);  // Se duerme
    }
}

// Reemplazo del Idle Hook
void vTaskBoton(void *pvParameters)
{
    bool ultimoEstadoBoton = false;
    
    while(1)
    {
        bool estadoBoton = (gpio_get_level(bot) == 1);
        
        // Detecta el momento exacto en que se presiona (flanco de subida)
        if(estadoBoton && !ultimoEstadoBoton)
        {
            xTimerStop(TiempoBloqueo, 0);
            ESP_LOGI(TAG, "Boton presionado, Mostrando adc ->");
            vTaskResume(Task_3);
        }
        ultimoEstadoBoton = estadoBoton;
        
        // Cede CPU para evitar bloqueos
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main() 
{
    gpio_reset_pin(bot);
    gpio_set_direction(bot, GPIO_MODE_INPUT);
    gpio_set_pull_mode(bot, GPIO_PULLDOWN_ONLY);

    gpio_reset_pin(Led);
    gpio_set_direction(Led, GPIO_MODE_OUTPUT);

    adc_reader_init();

    TiempoBloqueo = xTimerCreate("TiempoBloqueo", pdMS_TO_TICKS(5000), pdFALSE, NULL, vTimerCallback);

    // Creación de tareas
    xTaskCreate(vTaskLedRapido, "Led_Rapido",  2048, NULL, 3, &Task_1);
    xTaskCreate(vTaskLedLento,  "Led_Lento",   2048, NULL, 2, &Task_2);
    xTaskCreate(vTaskSensor,    "Sensor",      2048, NULL, 1, &Task_3);
    
    // Tarea de monitoreo del botón (Prioridad 4 para asegurar respuesta inmediata)
    xTaskCreate(vTaskBoton,     "Tarea_Boton", 2048, NULL, 4, NULL);

    // Se inicia el ciclo comenzando el timer
    xTimerStart(TiempoBloqueo, 0);
}