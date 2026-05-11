#include "hc06.h"

#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

bool hc06_check_connection() {
    char str[64];
    int i = 0;

    uart_puts(HC06_UART_ID, "AT");

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (uart_is_readable_within_us(HC06_UART_ID, 1200) && i < (int)(sizeof(str) - 1)) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        str[i++] = uart_getc(HC06_UART_ID);

    }
    str[i] = '\0';
    vTaskDelay(pdMS_TO_TICKS(2000));
    if (strstr(str, "OK") != NULL)
        return true;
    else
        return false;
}

bool hc06_set_name(char name[]) {
    char cmd[32];
    char str[64];
    int i = 0;

    snprintf(cmd, sizeof(cmd), "AT+NAME%s", name);
    uart_puts(HC06_UART_ID, cmd);

    while (uart_is_readable_within_us(HC06_UART_ID, 1200) && i < (int)(sizeof(str) - 1)) {
        str[i++] = uart_getc(HC06_UART_ID);
    }
    str[i] = '\0';


    if (strstr(str, "OKsetname") != NULL) {
        return true;
    } else {
        return false;
    }
}

bool hc06_set_pin(char pin[]) {
    char cmd[32];
    char str[64];
    int i = 0;

    // Monta o comando AT com o PIN
    snprintf(cmd, sizeof(cmd), "AT+PIN%s", pin);
    uart_puts(HC06_UART_ID, cmd);

    // Lê a resposta da UART, com verificação de limite
    while (uart_is_readable_within_us(HC06_UART_ID, 1200) && i < (int)(sizeof(str) - 1)) {
        str[i++] = uart_getc(HC06_UART_ID);
    }
    str[i] = '\0';

    // Verifica se a resposta contém "OK"
    if (strstr(str, "OKsetPIN") != NULL) {
        return true;
    } else {
        return false;
    }
}

bool hc06_set_baud_115200() {
    char str[64];
    int i = 0;

    uart_puts(HC06_UART_ID, "AT+BAUD8");

    while (uart_is_readable_within_us(HC06_UART_ID, 1200) && i < (int)(sizeof(str) - 1)) {
        str[i++] = uart_getc(HC06_UART_ID);
    }
    str[i] = '\0';

    if (strstr(str, "OK115200") != NULL)
        return true;
    else
        return false;
}

bool hc06_set_at_mode(int on){
    gpio_init(HC06_ENABLE_PIN);
    gpio_set_dir(HC06_ENABLE_PIN, GPIO_OUT);
    gpio_put(HC06_ENABLE_PIN, on);
    return true;
}



bool hc06_config(char name[], char pin[]) {

    uart_init(HC06_UART_ID, 9600);
    vTaskDelay(pdMS_TO_TICKS(1200));

    int tentativa = 0;

    hc06_set_at_mode(1);
    
    printf("\n\nBaud Rate = 9600\n");
    printf("Checando...\n");
    vTaskDelay(pdMS_TO_TICKS(1200));
    while (hc06_check_connection() == false) {
        printf("Nao Conectado\n");
        vTaskDelay(pdMS_TO_TICKS(1200));
        
        if(tentativa == 4){
            printf("\n\nBaud Bate = 115200\n"); 
            printf("Checando...\n");
            uart_init(HC06_UART_ID, 115200);
            vTaskDelay(pdMS_TO_TICKS(1200));
        }
        else if(tentativa == 9){
            printf("\n\nERRO!!!\n");
            printf("1 - Verique as conexoes com o modulo e reinicie.\n\n");
            printf("                    OU\n\n");
            printf("2 - Substitua o Modulo HC-06 e reinicie.\n\n\n");
            while(1);
        }

        tentativa++;
    }
    printf("Conectado!!!\n\n");
    
    vTaskDelay(pdMS_TO_TICKS(1200));
    if(tentativa <= 4){
        printf("\n\nAlterando Baud Rate do HC-06 para 115200\n");
        while (hc06_set_baud_115200() == false) {
            printf("set baud failed\n");
            vTaskDelay(pdMS_TO_TICKS(1200));
        }
        printf("0 - baud 115200 OK\n\n");
        uart_init(HC06_UART_ID, 115200);
    }

    vTaskDelay(pdMS_TO_TICKS(1200));
    printf("setting name\n");
    vTaskDelay(pdMS_TO_TICKS(1200));
    while (hc06_set_name(name) == false) {
        printf("set name failed\n");
        vTaskDelay(pdMS_TO_TICKS(1200));
    }
    printf("1 - name OK\n\n");

    vTaskDelay(pdMS_TO_TICKS(1200));
    printf("setting pin\n");
    vTaskDelay(pdMS_TO_TICKS(1200));
    while (hc06_set_pin(pin) == false) {
        printf("set pin failed\n");
        vTaskDelay(pdMS_TO_TICKS(1200));
    }
    printf("2 - pin OK\n\n");

    printf("HC-06 CONFIGURADO!!!\n");

    hc06_set_at_mode(0);
}
