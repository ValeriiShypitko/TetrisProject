# STM32F446 Bare-Metal Tetris

Tetris on four daisy-chained MAX7219 8x8 LED matrices, driven by an STM32F446RE
(NUCLEO-F446RE) with **no HAL, no CMSIS device headers, no RTOS** — all
peripheral access goes through register-level drivers in this repository.

I wrote the GPIO/SPI drivers while following the
[MCU1 course](https://www.udemy.com/course/mastering-microcontroller-with-peripheral-driver-development/)
(Mastering Microcontroller and Embedded Driver Development) to learn the
protocols, then extended them with my own additions: table-driven RCC
clock/reset control, SysTick/NVIC/SCB support, CS helpers, convenience init
functions, plus the MAX7219 display driver and the game itself, which are
entirely my own work.


## Hardware

| Part | Role |
|------|------|
| NUCLEO-F446RE | MCU board (Cortex-M4 @ 16 MHz HSI) |
| 4x MAX7219 8x8 LED matrix | 8x32 playfield, daisy-chained |
| 4x push buttons | move left/right, spin left/right |

### Wiring

| Signal | Pin | Notes |
|--------|-----|-------|
| SPI2 SCK | PB13 | AF5 |
| SPI2 MOSI | PB15 | AF5 → MAX7219 DIN |
| SPI2 MISO | PB14 | AF5, unused by the display |
| MAX7219 CS | PB8 | plain GPIO, software-controlled |
| Move left | PB9 | button to GND, internal pull-up, EXTI falling edge |
| Move right | PB5 | " |
| Spin left | PB7 | " |
| Spin right | PB6 | " |
| Score output | PA2 (USART2 TX) | 115200 baud via ST-LINK VCP, `printf` |

## Architecture

The interesting part is the interrupt design — a small deferred-processing
scheme built from Cortex-M4 exception priorities, the same idea an RTOS uses
for its scheduler:

```
button press ──► EXTI9_5 ISR      debounce, queue a move/spin intent (fast, no game logic)
SysTick 1 ms ──► SysTick_Handler  count time; every GAMESPEED ms pend PendSV
                 PendSV_Handler   (lowest priority) consume intents, apply gravity,
                                  collision-check, clear rows, redraw over SPI
main loop    ──► prints the score over UART, nothing time-critical
```

- Intents are consumed with an interrupt-masked read-and-clear (`CPSID`/`CPSIE`),
  so a button press can never be half-read.
- `EXTI->PR` is write-1-to-clear; the handler clears exactly the line it
  serviced and never read-modify-writes the register.
- The game runs entirely in PendSV at the lowest priority, so button and tick
  interrupts are never delayed by game logic or SPI traffic.
- Pieces are dealt from a 7-bag (Fisher-Yates shuffle, O(1) per draw), seeded
  by the SysTick count at the first button press.

### Layers

```
Src/main.c            board bring-up (pin mux, SPI2 config) + score reporting
Src/tetris.c          game logic + ISR handlers
drivers/Src/          reusable drivers, no board knowledge:
  stm32f446xx_gpio_driver.c   GPIO + EXTI configuration
  stm32f446xx_spi_driver.c    blocking SPI master TX/RX
  stm32f446xx_rcc.c           table-driven peripheral clock enable/reset
  max7219_driver.c            display driver, bound to a bus/CS via max7219_Config()
drivers/Inc/stm32f446xx.h     register definitions written from RM0390
```

## Building

Requires CMake ≥ 3.20, Ninja, and the GNU Arm toolchain (the presets point at
STM32CubeCLT's copy; edit `cmake/gnu-tools-for-stm32.cmake` for a different
install).

```sh
cmake --preset Debug        # or Release
cmake --build build/Debug
```

Flash `build/Debug/stm32f446-tetris.elf` with your preferred tool, e.g.:

```sh
STM32_Programmer_CLI -c port=SWD -w build/Debug/stm32f446-tetris.hex -v -rst
```

## Gameplay

- The game waits for any button press to start (the press also seeds the
  randomizer).
- Pieces fall every 270 ms (`GAMESPEED` in `Inc/tetris.h`).
- +75 points per tick survived, +1100 per cleared row.
- Score streams over USART2; game over clears the display and prints the
  final score.
