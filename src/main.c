// lib padrão do c
#include <stdio.h>

// lib da pico
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/adc.h"

// libs para utilização dos periféricos da placa
#include "../libs/display.h"
#include "../libs/matrix.h"
#include "../libs/button.h"

// macros
#define ADC_PIN 28
#define ADC_VREF 3.31
#define ADC_RESOLUTION 4095
#define KNOWN_RESISTOR 10000
#define E24_SIZE 24

#define REAL_VALUES 0

// variaves globais
float r_unknown = 0.f;
display dp;

// vetor da série E24 de resistores
const float E24_VALUES[] = {
    10, 11, 12, 13, 15, 16, 18, 20, 22, 24,
    27, 30, 33, 36, 39, 43, 47, 51, 56, 62,
    68, 75, 82, 91
};

// cores padrão
const rgb_led COLOR_TABLE[10] = {
    {0, 0, 0},         // 0 - Preto
    {139, 69, 19},     // 1 - Marrom
    {255, 0, 0},       // 2 - Vermelho
    {255, 165, 0},     // 3 - Laranja
    {255, 255, 0},     // 4 - Amarelo
    {0, 128, 0},       // 5 - Verde
    {0, 0, 255},       // 6 - Azul
    {128, 0, 128},     // 7 - Violeta
    {128, 128, 128},   // 8 - Cinza
    {255, 255, 255}    // 9 - Branco
};

// função para inicialização dos componentes
void setup();

// função para desenhar a tela inicial
void draw_screen(char *, char *, display *);

// Função para encontrar o valor comercial E24 mais próximo
float find_nearest_e24(float resistance);

// Função para obter as cores (em rgb_led) correspondentes ao valor
void get_resistor_colors(float resistance, rgb_led colors[3]);

int main() {
    stdio_init_all();
    setup();

    char adc_string[10];
    char resistor_string[10];

    while(true) { // talvez ter que colocar o adc_select_input aqui
        if (button_get_event() == BUTTON_JOYSTICK) {
            display_shutdown(&dp);
            matrix_clear();
            matrix_update();
            reset_usb_boot(0, 0);
        }

        float soma = 0.f, media;

        #if REAL_VALUES
            for (int i = 0; i < 500; i++) {
                soma += adc_read();
                sleep_ms(1);
            }
            media = soma / 500.f;
        #else
            media = 2047;
        #endif

        r_unknown = (KNOWN_RESISTOR * media) / (ADC_RESOLUTION - media);

        sprintf(adc_string, "%1.0f", media);
        sprintf(resistor_string, "%1.0f", r_unknown);

        draw_screen(adc_string, resistor_string, &dp);

        
        sleep_ms(100);
    }
}

void setup() {
    display_init(&dp);
    matrix_init(MATRIX_LED_PIN);
    button_init();
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(2);
}

void draw_screen(char *adc, char *resistor, display *dp) {
    display_clear(dp);

    display_draw_rectangle(3, 3, 122, 60, false, true, dp);

    display_draw_line(3, 25, 123, 25, true, dp);
    display_draw_line(3, 37, 123, 37, true, dp);

    display_draw_string(10, 28, "Ohmimetro", true, dp);
    display_draw_string(13, 41, "ADC", true, dp);
    display_draw_string(50, 41, "Resisten.", true, dp);

    display_draw_line(44, 37, 44, 60, true, dp);

    display_draw_string(8, 52, adc, true, dp);
    display_draw_string(59, 52, resistor, true, dp);

    display_update(dp);
}