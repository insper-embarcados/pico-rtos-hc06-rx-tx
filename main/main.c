#include "hc06.h"
#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "pico/stdlib.h"

#define HC06_NAME "aps2_legal"
#define HC06_PIN "1234"

static void init_task(void *p) {
    (void)p;

    printf("Iniciando configuracao do HC-06\n");
    if (!hc06_config(HC06_NAME, HC06_PIN)) {
        printf("Falha ao configurar o HC-06\n");
    } else {
        printf("HC-06 configurado com sucesso\n");
    }

    vTaskDelete(NULL);
}

int main() {
    stdio_init_all();

    xTaskCreate(init_task, "init_task", 4096, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true) {
        ;
    }
}
