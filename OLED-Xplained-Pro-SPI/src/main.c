/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

/************************************************************************/
/* botoes                                                               */
/************************************************************************/

// Configuracoes do Botao 1
#define BUT_PIO PIOD                 // periferico que controla o LED
#define BUT_PIO_ID ID_PIOD           // ID do periférico PIOC (controla Botao)
#define BUT_IDX 28                   // NO EXT = 9
#define BUT_IDX_MASK (1u << BUT_IDX) // Mascara para CONTROLARMOS o Botao

// Configuracoes do Botao 2
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_IDX 31 // NO EXT = 3
#define BUT2_IDX_MASK (1u << BUT2_IDX)

// Configuracoes do Botao 3
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_IDX 19 // NO EXT = 4
#define BUT3_IDX_MASK (1u << BUT3_IDX)

/************************************************************************/
/* LED                                                                  */
/************************************************************************/

// Configuracoes do LED
#define LED_PIO PIOA                // periferico que controla o LED
#define LED_PIO_ID ID_PIOA          // ID do periférico PIOC (controla LED)
#define LED_IDX 0                   // NO EXT = 7
#define LED_IDX_MASK (1 << LED_IDX) // Mascara para CONTROLARMOS o LED

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

volatile char but_pressed = 0;
volatile char but_was_pressed = 0;
volatile char pisca_pisca = 0;
volatile char but3_pressed = 0;
volatile char but3_was_pressed = 0;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void _but_callback(void);
void _but2_callback(void);
void _but3_callback(void);

void io_init(void);
char pisca_leds(char set, char counter);

int handle_but(int delay, int counter);
int handle_but3(int delay, int counter);

void update_display(int delay, char counter);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

void _but_callback(void) {
    if (!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)) {
        but_pressed = 1;
        pisca_pisca = 0;
    } else if (but_pressed) {
        but_pressed = 0;
        but_was_pressed = 1;
    }
}

void _but2_callback(void) {
    pisca_pisca = !pisca_pisca;
}

void _but3_callback(void) {
    if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK)) {
        but3_pressed = 1;
        pisca_pisca = 0;
    } else if (but3_pressed) {
        but3_pressed = 0;
        but3_was_pressed = 1;
    }
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Inicializa botao SW0 do kit com interrupcao
void io_init(void) {
    // Configura os leds
    pmc_enable_periph_clk(LED_PIO_ID);
    pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

    // Inicializa clock dos periféricos PIOs responsavelis pelos botoes
    pmc_enable_periph_clk(BUT_PIO_ID);
    pmc_enable_periph_clk(BUT2_PIO_ID);
    pmc_enable_periph_clk(BUT3_PIO_ID);

    // Configura PIO para lidar com o pino do botão como entrada
    // com pull-up
    pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
    pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP);
    pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP);

    // Configura interrupção no pino referente a cada botao e associa uma
    // função de callback para caso uma interrupção for gerada
    pio_handler_set(BUT_PIO,
                    BUT_PIO_ID,
                    BUT_IDX_MASK,
                    PIO_IT_EDGE,
                    _but_callback);

    pio_handler_set(BUT2_PIO,
                    BUT2_PIO_ID,
                    BUT2_IDX_MASK,
                    PIO_IT_RISE_EDGE,
                    _but2_callback);

    pio_handler_set(BUT3_PIO,
                    BUT3_PIO_ID,
                    BUT3_IDX_MASK,
                    PIO_IT_EDGE,
                    _but3_callback);

    // Ativa interrupção e limpa primeira IRQ gerada na ativacao de cada botao
    pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
    pio_get_interrupt_status(BUT_PIO);

    pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
    pio_get_interrupt_status(BUT2_PIO);

    pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);
    pio_get_interrupt_status(BUT3_PIO);

    // Configura NVIC para receber interrupcoes do PIO do botao
    // com prioridade 4 (quanto mais próximo de 0 maior)
    NVIC_EnableIRQ(BUT_PIO_ID);
    NVIC_SetPriority(BUT_PIO_ID, 5); // Prioridade 4

    NVIC_EnableIRQ(BUT2_PIO_ID);
    NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 5

    NVIC_EnableIRQ(BUT3_PIO_ID);
    NVIC_SetPriority(BUT3_PIO_ID, 6); // Prioridade 6
}

char pisca_leds(char set, char counter) {
}

int handle_but(int delay, int counter) {
    if (but_pressed) {
        counter++;
    } else if (but_was_pressed) {
        if (counter < 2500) {
            delay += 100;
        } else if (delay > 0) {
            delay -= 100;
        }
        counter = 0;
        but_was_pressed = 0;
    }
    return delay;
}

int handle_but3(int delay, int counter) {
    if (but3_pressed) {
        counter++;
    } else if (but3_was_pressed) {
        if (counter < 2500 && delay > 0) {
            delay -= 100;
        }
        counter = 0;
        but3_was_pressed = 0;
    }
    return delay;
}

void update_display(int delay, char counter) {
    char str[128];
    sprintf(str, "Freq : %dHz", 500 / delay);

    gfx_mono_draw_filled_rect(0, 16, 128, 8, GFX_PIXEL_CLR);

    gfx_mono_draw_string(str, 0, 0, &sysfont);

    for (int i = counter; i > 0; i--) {
        gfx_mono_draw_rect(4 + i * 4, 16, 4, 8, GFX_PIXEL_SET);
    }
}

int main(void) {
    board_init();
    sysclk_init();
    delay_init();

    // Init Buttons
    io_init();

    // Init OLED
    gfx_mono_ssd1306_init();

    int delay = 100;

    int timer = 0;
    char set = 0;
    char led_counter = 0;

    int but_counter = 0;
    int but3_counter = 0;

    pio_set(LED_PIO, LED_IDX_MASK);

    /* Insert application code here, after the board has been initialized. */
    while (1) {
        int originalDelay = delay;

        delay = handle_but(delay, but_counter);

        // Stop
        while (pisca_pisca) {
        }
        delay = handle_but3(delay, but3_counter);

        timer++;
        delay_ms(1);

        if (delay == 0) {
            timer = 0;
            set = 1;

            pio_clear(LED_PIO, LED_IDX_MASK);
        } else if (timer >= delay) {
            timer = 0;
            set = !set;

            if (set) {
                pio_clear(LED_PIO, LED_IDX_MASK);
            } else {
                pio_set(LED_PIO, LED_IDX_MASK);

                led_counter++;

                update_display(delay, led_counter);
            }
        } else if (originalDelay != delay) {
            update_display(delay, led_counter);
        }
        if (led_counter >= 30) {
            led_counter = 0;
            pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
        }
    }
}