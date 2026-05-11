#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hc06.h"

#define HC06_NAME "LAB-EXPERT-BT"
#define HC06_PIN  "1234"

#define QUEUE_SIZE 256

static QueueHandle_t xQueueRX;
static QueueHandle_t xQueueTX;
static TaskHandle_t  xRxTaskHandle;

static void hc06_rx_irq(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uart_set_irq_enables(HC06_UART_ID, false, false);
    vTaskNotifyGiveFromISR(xRxTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void init_task(void *p) {
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);

    hc06_config(HC06_NAME, HC06_PIN);

    uart_set_fifo_enabled(HC06_UART_ID, false);

    int uart_irq = HC06_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(uart_irq, hc06_rx_irq);
    irq_set_priority(uart_irq, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    irq_set_enabled(uart_irq, true);
    uart_set_irq_enables(HC06_UART_ID, true, false);

    vTaskDelete(NULL);
}

static void rx_task(void *p) {
    xRxTaskHandle = xTaskGetCurrentTaskHandle();

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (uart_is_readable(HC06_UART_ID)) {
            uint8_t ch = uart_getc(HC06_UART_ID);
            xQueueSend(xQueueRX, &ch, 0);
        }

        uart_set_irq_enables(HC06_UART_ID, true, false);
    }
}

static void tx_task(void *p) {
    uint8_t ch;
    while (true) {
        xQueueReceive(xQueueTX, &ch, portMAX_DELAY);
        uart_putc_raw(HC06_UART_ID, ch);
    }
}

static void serial_task(void *p) {
    uint8_t ch;
    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            ch = (uint8_t)c;
            xQueueSend(xQueueTX, &ch, 0);
        }

        while (xQueueReceive(xQueueRX, &ch, 0) == pdTRUE) {
            putchar_raw(ch);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void) {
    stdio_init_all();

    xQueueRX = xQueueCreate(QUEUE_SIZE, sizeof(uint8_t));
    xQueueTX = xQueueCreate(QUEUE_SIZE, sizeof(uint8_t));

    xTaskCreate(init_task,   "Init",   2048, NULL, 3, NULL);
    xTaskCreate(rx_task,     "RX",     512,  NULL, 2, NULL);
    xTaskCreate(tx_task,     "TX",     512,  NULL, 2, NULL);
    xTaskCreate(serial_task, "Serial", 1024, NULL, 1, NULL);

    vTaskStartScheduler();
    while (true);
}
