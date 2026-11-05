#include "hc06.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#define HC06_TIMEOUT_RESPOSTA_US 1200
#define HC06_ESPERA_AT_MS        500
#define HC06_ESPERA_CMD_MS       1500
#define HC06_INTERVALO_CMD_MS    5000
#define HC06_TENTATIVAS_POR_BAUD 3

static void hc06_limpar_rx(void) {
    while (uart_is_readable_within_us(HC06_UART_ID, HC06_TIMEOUT_RESPOSTA_US))
        uart_getc(HC06_UART_ID);
}

static int hc06_ler_resposta(char *buf, size_t len) {
    int i = 0;
    while (uart_is_readable_within_us(HC06_UART_ID, HC06_TIMEOUT_RESPOSTA_US) && i < (int)(len - 1)) {
        buf[i++] = uart_getc(HC06_UART_ID);
    }
    buf[i] = '\0';
    return i;
}

bool hc06_check_connection() {
    char str[64];
    uart_puts(HC06_UART_ID, "AT");
    sleep_ms(HC06_ESPERA_AT_MS);
    hc06_ler_resposta(str, sizeof(str));
    return strstr(str, "OK") != NULL;
}

bool hc06_get_version(char version[], size_t len) {
    if (version == NULL || len == 0) return false;

    hc06_limpar_rx();
    uart_puts(HC06_UART_ID, "AT+VERSION");
    sleep_ms(HC06_ESPERA_CMD_MS);
    return hc06_ler_resposta(version, len) > 0;
}

bool hc06_set_name(char name[]) {
    char cmd[32];
    char str[64];
    snprintf(cmd, sizeof(cmd), "AT+NAME%s", name);
    uart_puts(HC06_UART_ID, cmd);
    sleep_ms(HC06_ESPERA_CMD_MS);
    hc06_ler_resposta(str, sizeof(str));
    return strstr(str, "OKsetname") != NULL;
}

bool hc06_set_pin(char pin[]) {
    char cmd[32];
    char str[64];
    snprintf(cmd, sizeof(cmd), "AT+PIN%s", pin);
    uart_puts(HC06_UART_ID, cmd);
    sleep_ms(HC06_ESPERA_CMD_MS);
    hc06_ler_resposta(str, sizeof(str));
    return strstr(str, "OKsetPIN") != NULL;
}

bool hc06_set_baud_115200() {
    char str[64];
    uart_puts(HC06_UART_ID, "AT+BAUD8");
    sleep_ms(HC06_ESPERA_AT_MS);
    hc06_ler_resposta(str, sizeof(str));
    return strstr(str, "OK115200") != NULL;
}

static bool hc06_tentar_baud(uint baud) {
    printf("Tentando baud = %u...\n", baud);
    uart_set_baudrate(HC06_UART_ID, baud);
    sleep_ms(HC06_ESPERA_AT_MS);

    for (int tentativa = 1; tentativa <= HC06_TENTATIVAS_POR_BAUD; tentativa++) {
        if (hc06_check_connection()) return true;
        printf("baud = %u nao respondeu (tentativa %d/%d)\n",
               baud, tentativa, HC06_TENTATIVAS_POR_BAUD);
        if (tentativa < HC06_TENTATIVAS_POR_BAUD) {
            sleep_ms(HC06_INTERVALO_CMD_MS);
        }
    }

    return false;
}

bool hc06_config(char name[], char pin[]) {
    bool connected = false;
    bool already_115200 = false;

    // Pico e HC-06 recebem alimentacao ao mesmo tempo. Aguarda o firmware do
    // modulo terminar a inicializacao antes do primeiro comando AT.
    printf("Aguardando inicializacao do HC-06...\n");
    sleep_ms(HC06_INTERVALO_CMD_MS);

    if (hc06_tentar_baud(115200)) {
        connected = true;
        already_115200 = true;
    } else {
        sleep_ms(HC06_INTERVALO_CMD_MS);
        if (hc06_tentar_baud(9600)) {
            connected = true;
            already_115200 = false;
        }
    }

    if (!connected) {
        printf("ERRO: HC-06 nao respondeu aos comandos AT.\n");
        uart_set_baudrate(HC06_UART_ID, HC06_BAUD_RATE);
        return false;
    }

    printf("Conectado!\n\n");
    sleep_ms(HC06_INTERVALO_CMD_MS);

    char version[64];
    printf("Verificando firmware...\n");
    if (!hc06_get_version(version, sizeof(version))) {
        printf("ERRO: HC-06 nao respondeu ao comando AT+VERSION.\n");
        return false;
    }

    printf("Firmware detectado: %s\n", version);
    if (strcmp(version, HC06_EXPECTED_VERSION) != 0) {
        printf("ERRO: firmware incompativel. Esperado: %s\n",
               HC06_EXPECTED_VERSION);
        return false;
    }
    printf("Firmware compativel.\n\n");
    sleep_ms(HC06_INTERVALO_CMD_MS);

    if (!already_115200) {
        printf("Alterando baud rate para 115200...\n");
        if (!hc06_set_baud_115200()) {
            printf("set baud failed\n");
            return false;
        }
        uart_init(HC06_UART_ID, 115200);
        sleep_ms(HC06_INTERVALO_CMD_MS);
        hc06_limpar_rx();
        printf("Baud rate OK\n\n");
    }

    printf("Configurando nome...\n");
    if (!hc06_set_name(name)) {
        printf("set name failed\n");
        return false;
    }
    printf("Nome OK\n\n");
    sleep_ms(HC06_INTERVALO_CMD_MS);

    printf("Configurando PIN...\n");
    if (!hc06_set_pin(pin)) {
        printf("set pin failed\n");
        return false;
    }
    printf("PIN OK\n\n");

    printf("HC-06 configurado!\n");
    return true;
}
