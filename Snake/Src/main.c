#include "stm32f103xb.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDTH  90
#define HEIGHT 40
#define SR_TXE    (1U << 7)

volatile uint32_t ms_ticks = 0;

void SysTick_Handler(void) {
    ms_ticks++;
}

uint32_t get_ticks(void) {
    return ms_ticks;
}

typedef struct { int x, y; } Point;
Point snake[100];
int snake_len = 3;
Point food;
char dir = 'r';
int score = 0;

void uart1_write(char ch) {
    while (!(USART1->SR & SR_TXE));
    USART1->DR = (ch & 0xFF);
}

void uart_write(const char *s) {
    while (*s) uart1_write(*s++);
}

void uart_init(void) {
    RCC->APB2ENR |= (1 << 2) | (1 << 14); // Port A, USART1
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);
    GPIOA->CRL &= ~(0xFFFF);
    GPIOA->CRL |=   0x8888;
    GPIOA->ODR |=   0x000F;

    USART1->BRR = 0x0341;
    USART1->CR1 |= (1U << 13) | (1U << 3);
}

void draw() {
    char screen_buf[(WIDTH + 5) * (HEIGHT + 5) + 128];
    char *p = screen_buf;

    p += sprintf(p, "\033[H SCORE: %d    LENGTH: %d \r\n", score, snake_len);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) *p++ = '#';
            else if (x == snake[0].x && y == snake[0].y) *p++ = 'O';
            else if (x == food.x && y == food.y) *p++ = '@';
            else {
                int isBody = 0;
                for (int i = 1; i < snake_len; i++) {
                    if (x == snake[i].x && y == snake[i].y) {
                        *p++ = 'o';
                        isBody = 1; break;
                    }
                }
                if (!isBody) *p++ = ' ';
            }
        }
        *p++ = '\r'; *p++ = '\n';
    }
    *p = '\0';
    uart_write(screen_buf);
}

void update() {
    Point nextHead = snake[0];
    if (dir == 'u') nextHead.y--;
    if (dir == 'd') nextHead.y++;
    if (dir == 'l') nextHead.x--;
    if (dir == 'r') nextHead.x++;
    for (int i = 0; i < snake_len; i++) {
        if (nextHead.x == snake[i].x && nextHead.y == snake[i].y) {
            score = 0;
            snake_len = 3;
            return;
        }
    }

    for (int i = snake_len - 1; i > 0; i--) snake[i] = snake[i - 1];
    snake[0] = nextHead;
    if (snake[0].x >= WIDTH - 1) snake[0].x = 1;
    else if (snake[0].x <= 0) snake[0].x = WIDTH - 2;
    if (snake[0].y >= HEIGHT - 1) snake[0].y = 1;
    else if (snake[0].y <= 0) snake[0].y = HEIGHT - 2;

    if (snake[0].x == food.x && snake[0].y == food.y) {
        score += 10;
        if (snake_len < 100) snake_len++;
        food.x = (rand() % (WIDTH - 2)) + 1;
        food.y = (rand() % (HEIGHT - 2)) + 1;
    }
}

int main(void) {
    SysTick_Config(8000000 / 1000);


    uart_init();
    uart_write("\033[?25l");
    uart_write("\033[2J");
    uint32_t last_move_time = 0;
    uint32_t move_interval = 200;

    snake[0] = (Point){WIDTH/2, HEIGHT/2};
    snake[1] = (Point){WIDTH/2 - 1, HEIGHT/2};
    snake[2] = (Point){WIDTH/2 - 2, HEIGHT/2};
    food.x = 10; food.y = 10;

    while (1) {
        if (!(GPIOA->IDR & (1 << 0)) && dir != 'd') dir = 'u';
        else if (!(GPIOA->IDR & (1 << 1)) && dir != 'u') dir = 'd';
        else if (!(GPIOA->IDR & (1 << 2)) && dir != 'r') dir = 'l';
        else if (!(GPIOA->IDR & (1 << 3)) && dir != 'l') dir = 'r';
        if ((get_ticks() - last_move_time) >= move_interval) {
            update();
            draw();
            last_move_time = get_ticks();
            move_interval = 200 - (score / 2);
            if (move_interval < 50) move_interval = 50;
        }
    }
}
