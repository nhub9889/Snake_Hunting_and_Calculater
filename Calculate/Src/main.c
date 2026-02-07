#include "stm32f103xb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SR_TXE (1U << 7)

char keys[4][4] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'0', '/', '%', '='}
};

int current_result = 0;
char last_op = 0;
char input_buffer[16] = {0};
int idx = 0;
int flag = 1;

void uart1_init(void) {
    RCC->APB2ENR |= (1 << 2) | (1 << 14);
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);
    USART1->BRR = 0x0341;
    USART1->CR1 |= (1 << 13) | (1 << 3);
}

void uart1_write_str(const char *s) {
    while (*s) {
        while (!(USART1->SR & SR_TXE));
        USART1->DR = (*s++ & 0xFF);
    }
}

void keypad_init(void) {
    GPIOA->CRL &= ~(0xFFFFFFFF);
    GPIOA->CRL |= 0x88883333;
    GPIOA->ODR |= 0xF0;
}

char keypad_scan(void) {
    for (int r = 0; r < 4; r++) {
        GPIOA->ODR |= 0x0F;
        GPIOA->ODR &= ~(1 << r);

        for (int c = 0; c < 4; c++) {
            if (!(GPIOA->IDR & (1 << (c + 4)))) {
                for(volatile int d = 0; d < 50000; d++);
                while (!(GPIOA->IDR & (1 << (c + 4))));
                return keys[r][c];
            }
        }
    }
    return 0;
}

int calculate(int v1, int v2, char op) {
    switch(op) {
        case '+': return v1 + v2;
        case '-': return v1 - v2;
        case '*': return v1 * v2;
        case '/': return (v2 != 0) ? v1 / v2 : 0;
        case '%': return (v2 != 0) ? v1 % v2 : 0;
        default: return v2;
    }
}

int main(void) {
    uart1_init();
    keypad_init();

    uart1_write_str("\r\n--- Calculator ---\r\n");

    while (1) {
        char key = keypad_scan();
        if (key) {
            char echo[2] = {key, '\0'};
            uart1_write_str(echo);

            if (key >= '0' && key <= '9') {
                if (idx < 15) input_buffer[idx++] = key;
            }
            else {
                input_buffer[idx] = '\0';
                int current_val = atoi(input_buffer);
                idx = 0;

                if (flag) {
                    current_result = current_val;
                    flag = 0;
                } else {
                    current_result = calculate(current_result, current_val, last_op);
                }

                if (key == '=') {
                    char res_str[40];
                    sprintf(res_str, "\r\nResult: %d\r\n", current_result);
                    uart1_write_str(res_str);
                    flag = 1;
                    last_op = 0;
                } else {
                    last_op = key;
                }
                memset(input_buffer, 0, 16);
            }
        }
    }
}
