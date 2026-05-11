# pico-rtos-hc06-rx-tx

Sistema de inicialização e configuração do módulo **HC-06** em uma **Raspberry Pi Pico 2** com **FreeRTOS**.

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

---

## Organização dos arquivos

- `main/hc06.h` / `main/hc06.c`: Configuração do HC-06 via comandos AT.
- `main/main.c`: Task de inicialização que configura nome e PIN do módulo.

---

## Task de init

A task `init_task` chama `hc06_config("aps2_legal", "1234")` e deixa o módulo pronto com o nome e PIN definidos no código.

---

## Como compilar (Pico)

```bash
mkdir build && cd build
cmake ..
make -j4
```

Segure **BOOTSEL**, conecte via USB e arraste o arquivo `build/pico_emb.uf2` para a unidade que aparecer.

---

## Como executar

1. Compile o projeto
2. Grave o `.uf2` na Pico
3. Abra o monitor serial para ver a inicialização do HC-06
