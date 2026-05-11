# pico-rtos-hc06-rx-tx

Sistema de comunicação bidirecional Bluetooth entre uma **Raspberry Pi Pico 2** (FreeRTOS) e uma **interface gráfica Python (Tkinter)**, utilizando o módulo **HC-06** como meio de transmissão.

Manual HC-06: https://www.olimex.com/Products/Components/RF/BLUETOOTH-SERIAL-HC-06/resources/hc06.pdf

---

## Conexões

### HC-06 → Pico

| HC-06  | Pico       |
|--------|------------|
| STATE  | GP3        |
| RXD    | GP4 (TX1)  |
| TXD    | GP5 (RX1)  |
| ENABLE | GP6        |
| GND    | GND        |
| VCC    | VBUS (5V)  |

### LCD I2C → Pico

| LCD I2C | Pico  |
|---------|-------|
| SDA     | GP14  |
| SCL     | GP15  |
| GND     | GND   |
| VCC     | 3.3V  |

### Periféricos

| Componente | Pino |
|------------|------|
| Botão      | GP15 |
| LED        | GP16 |

---

## Organização dos arquivos

- `main/hc06.h` / `main/hc06.c`: Configuração e inicialização do módulo HC-06 (pinos, UART, comandos AT).
- `main/lcd_1602_i2c.h` / `main/lcd_1602_i2c.c`: Driver do display LCD I2C 16x2.
- `main/main.c`: Tarefas FreeRTOS, IRQ UART, parser do protocolo binário, controle de LED e botão.
- `python/main.py`: Interface gráfica Tkinter — conecta via serial Bluetooth, exibe estado do LED e botão, envia texto ao LCD.

---

## Tarefas FreeRTOS

| Tarefa            | Responsabilidade |
|-------------------|-----------------|
| `hc06_task`       | Inicializa UART1 e habilita IRQ de RX |
| `led_task`        | Controla LED via botão físico (toggle) |
| `envia_protocolo` | Envia estado atual (botão + LED) a cada 200ms |
| `task_lcd_16x2`   | Processa pacotes recebidos e aciona LCD ou LED |

---

## Protocolo binário

Todos os pacotes começam com `0x00` (START) e terminam com `0xFF` (END).

| Comando                 | Valor  | Direção       | Descrição                    |
|-------------------------|--------|---------------|------------------------------|
| `PROTO_BUTTON_RELEASED` | `0x10` | Pico → Python | Botão solto                  |
| `PROTO_BUTTON_PRESSED`  | `0x11` | Pico → Python | Botão pressionado            |
| `PROTO_LED_OFF`         | `0x20` | Pico → Python | LED apagado                  |
| `PROTO_LED_ON`          | `0x21` | Pico → Python | LED aceso                    |
| `PROTO_LED_TOGGLE`      | `0x22` | Python → Pico | Inverter estado do LED       |
| `PROTO_LCD_TEXT`        | `0x30` | Python → Pico | Enviar texto para o LCD      |

---

## Como compilar (Pico)

```bash
mkdir build && cd build
cmake ..
make -j4
```

Segure **BOOTSEL**, conecte via USB e arraste o arquivo `build/pico_emb.uf2` para a unidade que aparecer.

---

## Como executar a interface Python

```bash
pip install pyserial
cd python
python main.py
```

1. Pareie o HC-06 no Bluetooth do sistema (PIN padrão: `1234`)
2. Selecione a porta COM e clique em **Conectar**

---

## Tutorial completo

Veja [`lab-expert-bluetooth/README.md`](lab-expert-bluetooth/README.md) para o tutorial detalhado com diagramas de arquitetura, protocolo e máquina de estados.
