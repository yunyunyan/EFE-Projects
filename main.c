#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "driver/uart.h"

#define LED_GPIO    GPIO_NUM_2
#define SWITCH_GPIO GPIO_NUM_0
#define UART_PORT   UART_NUM_0

SemaphoreHandle_t semSwitch = NULL;
SemaphoreHandle_t semUART = NULL;

#define BUF_SIZE (1024)
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_PORT, &uart_config); 
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0); 
}


static void Task1(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100);

    while (1) {
        printf("Tsk1-P1 <-\n");
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        printf("Tsk1-P1 ->\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void Task2(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(10);

    while (1) {
        printf("Tsk2-P2 <-\n");
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        printf("Tsk2-P2 ->\n");
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void Task3(void *pvParameters) {
    char rx_data[1];
    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        printf("Tsk3-P3 <-\n");
        xSemaphoreTake(semUART, portMAX_DELAY);
        uart_read_bytes(UART_PORT, rx_data, sizeof(rx_data), portMAX_DELAY);
        printf("Received character: %c\n", rx_data[0]);
        if (rx_data[0] == 'L' || rx_data[0] == 'l') {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(50));
            gpio_set_level(LED_GPIO, 0);
        }
        printf("Tsk3-P3 ->\n");
        xSemaphoreGive(semUART);
    }
}

static void Task4(void *pvParameters) {
    gpio_pad_select_gpio(SWITCH_GPIO);
    gpio_set_direction(SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_GPIO, GPIO_PULLUP_ONLY);

    while (1) {
        printf("Tsk4-P4 <-\n");
        xSemaphoreTake(semSwitch, portMAX_DELAY);
        printf("Switch pressed\n");
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("Tsk4-P4 ->\n");
        xSemaphoreGive(semSwitch);
    }
}

void app_main() {
    semSwitch = xSemaphoreCreateBinary();
    semUART = xSemaphoreCreateMutex();

    xSemaphoreGive(semSwitch);
    xSemaphoreGive(semUART);

    init_uart();

    xTaskCreate(Task1, "Task1", 4096, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 4096, NULL, 2, NULL);
    xTaskCreate(Task3, "Task3", 4096, NULL, 3, NULL);
    xTaskCreate(Task4, "Task4", 4096, NULL, 4, NULL);
}

