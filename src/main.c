// lib padrão do c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

#define REAL_VALUES 1

#define DEBUG(val) printf("%s: %d\n", #val, val)

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
    {20, 5, 0},        // 1 - Marrom
    {150, 0, 0},       // 2 - Vermelho
    {246, 20, 0},      // 3 - Laranja
    {150, 100, 0},     // 4 - Amarelo
    {0, 128, 0},       // 5 - Verde
    {0, 0, 128},       // 6 - Azul
    {128, 0, 80},      // 7 - Violeta
    {30, 30, 30},      // 8 - Cinza
    {150, 150, 150}    // 9 - Branco
};

const char *COLOR_NAMES[10] = {
    "Preto", "Marrom", "Vermelho", "Laranja",
    "Amarelo", "Verde", "Azul", "Violeta",
    "Cinza", "Branco"
};

int color_index[3];

// função para inicialização dos componentes
void setup();

// função para desenhar a tela inicial
void draw_screen(char *, char *, display *);

// função para encontrar o valor comercial E24 mais próximo e a cor
float find_nearest_e24(float resistance);
void get_resistor_colors(float resistance, rgb_led colors[3]);

int main() {
    stdio_init_all();
    setup();

    char adc_string[10];
    char resistor_string[10];

    while(true) {

        // botão B para entrar em bootsel, visando evitar apertar o botão atras da placa
        if (button_get_event() == BUTTON_B) {
            display_shutdown(&dp);
            matrix_clear();
            matrix_update();
            reset_usb_boot(0, 0);
        }

        // coleta dos dados para pelo adc / simulação de dados para depuração
        float soma = 0.f, media = 0;

        #if REAL_VALUES
            for (int i = 0; i < 500; i++) {
                soma += adc_read();
                sleep_ms(1);
            }
            media = soma / 500.f;
        #else
            media = 2047;
        #endif
        
        // calculo da resistencia do resistor desconhecido
        r_unknown = (KNOWN_RESISTOR * media) / (ADC_RESOLUTION - media);

        // coloca o valor no monitor serial
        DEBUG((int)r_unknown);
        
        // Pegar o valor mais proximo seguindo a tablela e24
        float r_e24 = find_nearest_e24(r_unknown);
        rgb_led colors[3];

        // conseguir as cores
        get_resistor_colors(r_e24, colors);

        sprintf(adc_string, "%1.0f", media);
        sprintf(resistor_string, "%1.0f", r_e24);
        
        // função que desenha tudo na tela
        draw_screen(adc_string, resistor_string, &dp);
        
        // lógica para printar a matriz
        matrix_clear();
        matrix_set_led_xy(2, 4, colors[0]);
        matrix_set_led_xy(2, 3, colors[1]);
        matrix_set_led_xy(2, 2, colors[2]);
        matrix_update();
        
        printf("resistor apos conversao: %.0f\n", r_e24);

        sleep_ms(1000);
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

    display_draw_string(37, 7, COLOR_NAMES[color_index[0]], true, dp);
    display_draw_string(37, 17, COLOR_NAMES[color_index[1]], true, dp);
    display_draw_string(37, 27, COLOR_NAMES[color_index[2]], true, dp);

    display_draw_line(3, 37, 123, 37, true, dp);

    display_draw_string(13, 41, "ADC", true, dp);
    display_draw_string(50, 41, "Resisten.", true, dp);

    display_draw_line(44, 37, 44, 60, true, dp);

    display_draw_string(8, 52, adc, true, dp);
    display_draw_string(59, 52, resistor, true, dp);

    display_update(dp);
}

float find_nearest_e24(float resistance) {
    float min_diff = 1e9;
    float nearest = 0;

    // procura o valor E24 mais próximo em todas as escalas
    for (int exp = -1; exp <= 6; exp++) { // de 0.1 a 10MΩ, pode ajustar se quiser
        float factor = powf(10, exp);
        for (int i = 0; i < 24; i++) {
            float candidate = E24_VALUES[i] * factor;
            float diff = fabsf(resistance - candidate);
            if (diff < min_diff) {
                min_diff = diff;
                nearest = candidate;
            }
        }
    }
    // DEBUG(nearest);
    return nearest;
}


// gera as três cores correspondentes
void get_resistor_colors(float resistance, rgb_led colors[3]) {
    int first_digit, second_digit, multiplier = 0;

    // normaliza para dois dígitos
    while (resistance >= 100) {
        resistance /= 10;
        multiplier++;
    }
    while (resistance < 10) {
        resistance *= 10;
        multiplier--;
    }

    int value = (int)(resistance + 0.5); // arredonda

    first_digit = value / 10;
    second_digit = value % 10;

    // Atribui as cores
    // DEBUG(first_digit);
    // DEBUG(second_digit);
    // DEBUG(multiplier);

    color_index[0] = first_digit;
    color_index[1] = second_digit;
    color_index[2] = multiplier;

    colors[0] = COLOR_TABLE[first_digit];
    colors[1] = COLOR_TABLE[second_digit];
    colors[2] = COLOR_TABLE[multiplier];
}
