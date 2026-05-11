# HC-06 Exemplo

Sistema de inicialização e configuração do módulo **HC-06** em uma **Raspberry Pi Pico 2** com **FreeRTOS**.

Manual: https://www.olimex.com/Products/Components/RF/BLUETOOTH-SERIAL-HC-06/resources/hc06.pdf

## No VS Code

Abra o projeto na pasta raiz e use o comando `Debug Project` da extensao Raspberry Pi Pico Project.

## Conexões

<table>
  <tr>
    <td valign="top">

### HC-06 → Pico

| HC-06  | Pico      |
|--------|-----------|
| STATE  | GP3       |
| RXD    | GP4 (TX1) |
| TXD    | GP5 (RX1) |
| ENABLE | GP6       |
| GND    | GND       |
| VCC    | VBUS (5V) |

    </td>
    <td valign="top">

![Diagrama de conexões de hardware](lab-expert-bluetooth/imgs/wiring.svg)

    </td>
  </tr>
</table>

## Organização dos arquivos

O projeto está organizado da seguinte maneira:

## Diagrama de tarefas e filas (RTOS)

### O que é e como funciona xTaskNotify

`xTaskNotify` é uma função da FreeRTOS usada para enviar notificações ou pequenos sinais diretamente entre tarefas (tasks) ou de uma rotina de interrupção (ISR) para uma task. O mecanismo é leve e eficiente, permitindo que uma task seja avisada rapidamente sobre eventos ou dados disponíveis, sem a necessidade de utilizar filas ou semáforos para casos simples.

No funcionamento típico, uma ISR (por exemplo, de UART) chama `xTaskNotify` (ou variantes como `vTaskNotifyGiveFromISR`) para alertar uma task de que há trabalho a ser feito. A task pode então esperar por essa notificação usando funções como `ulTaskNotifyTake`, processando apenas quando realmente necessário. Isso reduz o tempo gasto em interrupções e mantém o sistema mais responsivo.

**Resumo:**
- Notificações são leves e rápidas.
- Ideal para sinalizar eventos simples de ISR para tarefas.
- A task pode “dormir” até receber a notificação, acordando apenas quando houver necessidade de atendimento ao evento.

#### Diagrama resumido do funcionamento do xTaskNotify

```
 (UART_IRQ)
     o
     |
     | xTaskNotifyGiveFromISR
     v
+-----------+
| rx_task   |
| ulTaskNotifyTake() [aguardando notificação]
+-----------+
```
Legenda:
- (UART_IRQ): rotina de interrupção
- xTaskNotifyGiveFromISR: função chamada na ISR para sinalizar evento
- rx_task: task aguardando notificação


Utilizando o padrão recomendado pelo Insper para diagramação de sistemas embarcados com RTOS:

```
 (UART_IRQ)
     o
     |
     | xTaskNotify
     v
+---------+   xQueueRX    +-------------+         xQueueTX    +---------+
| rx_task | <------------ | serial_task | ------------------> | tx_task |
+---------+               +-------------+                     +---------+
```
Legenda:
- (UART_IRQ): interrupção, círculo = ISR
- quadrados: tasks FreeRTOS
- setas: direção e nome das filas usadas

- **rx_task**: É notificada pela interrupção, lê da UART e insere bytes na fila xQueueRX.
- **serial_task**: Faz a ponte entre o PC (via USB/serial), lê xQueueRX e mostra no PC; lê input do PC e coloca na xQueueTX.
- **tx_task**: Lê bytes da xQueueTX e envia via UART ao Bluetooth.
- **init_task**: (não aparece no fluxo, faz só a inicialização)

Esse fluxo garante comunicação assíncrona entre PC e HC-06 usando FreeRTOS conforme a notação do curso.


- `hc06.h`: Arquivo de headfile com configurações do HC06, tais como pinos e uart.

- `hc06.c`: Arquivo `.c` com implementação das funções auxiliares para configurar o módulo bluetooth:

```c
// Definições dos pinos e UART
#define HC06_UART_ID    uart1
#define HC06_BAUD_RATE  115200
#define HC06_STATE_PIN  3
#define HC06_RX_PIN     4
#define HC06_TX_PIN     5
#define HC06_ENABLE_PIN 6

// Protótipos das funções do HC-06
bool hc06_check_connection();
bool hc06_set_name(char name[]);
bool hc06_set_pin(char pin[]);
bool hc06_set_baud_115200();
bool hc06_set_at_mode(int on);
bool hc06_config(char name[], char pin[]);
```

- `main.c`: Arquivo principal com a task de inicialização do módulo bluetooth.

```c
// Task de inicialização: só roda uma vez no sistema
static void init_task(void *p) {
    // Inicializa GPIOs de UART, configura pinos e handler de IRQ
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);

    hc06_config(HC06_NAME, HC06_PIN); // configura nome, PIN e baud rate

    // IRQ da UART é configurada dentro da hc06_config
    vTaskDelete(NULL); // encerra a própria task após inicialização
}
```

No `main.c`, os valores de nome e PIN do módulo são definidos por macros:

```c
#define HC06_NAME "LAB-EXPERT-BT"
#define HC06_PIN  "1234"
```
Basta alterar essas linhas no `main.c` para personalizar o nome Bluetooth e PIN.

Extra ao que foi feito em sala de aula, eu adicionei o `hc06_set_at_mode`, que força o módulo bluetooth a entrar em modo AT; caso contrário, ele fica conectado ao equipamento e não recebe mais comandos.

## No Linux

Para testar a comunicação Bluetooth no Linux, siga a configuração do HC-06 usada em aula e faça o pareamento normalmente pelo gerenciador de Bluetooth do sistema.
