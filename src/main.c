#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

void PWM_init(void){
    DDRB |= (1 << PB3);
    TCCR0 |= (1 << WGM00) | (1 << WGM01) | (1 << COM01) | (1 << CS01);
}

void PWM_speed(uint8_t duty){
    OCR0 = duty;
}

void ADC_init(void){
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint16_t ADC_read(uint8_t channel){
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA = (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

void UART_init(void){
    UBRRH = 0;
    UBRRL = 103;
    UCSRB = (1 << TXEN) | (1 << RXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void UART_sendChar(char data){
    while(!(UCSRA & (1 << UDRE)));
    UDR = data;
}

void UART_sendString(const char *str){
    while(*str){
        UART_sendChar(*str++);
    }
}

void UART_sendINT(int number){
    char buffer[10];
    uint8_t i = 0;

    if(number == 0){
        UART_sendChar('0');
        return;
    }

    while(number){
        buffer[i++] = number % 10 + '0';
        number /= 10;
    }
    while(i){
        UART_sendChar(buffer[--i]);
    }
}

static void lcd_pulse_enable(void){
    PORTC |= (1 << PC1);
    _delay_us(1);

    PORTC &= ~(1 << PC1);
    _delay_us(50);
}

void LCD_cmd(uint16_t cmd){
    PORTC = (PORTC & 0x0F) | (cmd & 0xF0);
    PORTC &= ~(1 << PC0);
    lcd_pulse_enable();

    PORTC = (PORTC & 0x0F) | ((cmd << 4) & 0xF0);
    PORTC &= ~(1 << PC0);
    lcd_pulse_enable();
}

void LCD_data(uint16_t data){
    PORTC = (PORTC & 0x0F) | (data & 0xF0);
    PORTC &= ~(1 << PC0);
    lcd_pulse_enable();

    PORTC = (PORTC & 0x0F) | ((data << 4) & 0xF0);
    PORTC &= ~(1 << PC0);
    lcd_pulse_enable();
}

void LCD_init(void){
    DDRC |= 0x3F;

    _delay_ms(20);

    LCD_cmd(0x02);
    _delay_ms(2);
    LCD_cmd(0x28);
    LCD_cmd(0x0C);
    LCD_cmd(0x06);
    LCD_cmd(0x01);
    _delay_ms(2);
}

void LCD_string(const char *str){
    while(*str){
        LCD_data(*str++);
    }
}

void LCD_goto(uint8_t row, uint8_t col) {
    uint8_t pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_cmd(pos);
}

void LCD_integer(int number){
    char buffer[10];
    uint8_t i = 0;

    if(number == 0){
        LCD_data('0');
        return;
    }

    while(number){
        buffer[i++] = number % 10 + '0';
        number /= 10;
    }

    while(i){
        LCD_data(buffer[--i]);
    }
}

static volatile uint16_t temperature = 0;
static volatile uint8_t current_speed = 0;
static volatile uint8_t target_speed = 0;

void vSensorTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        uint16_t adc_val = ADC_read(0);
        uint16_t temp = (adc_val * 500UL) / 1024;

        taskENTER_CRITICAL();
        temperature = temp;
        taskEXIT_CRITICAL();

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vControlTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        uint16_t temp;

        taskENTER_CRITICAL();
        temp = temperature;
        taskEXIT_CRITICAL();

        uint8_t over60 = 0, over80 = 0, target = 0;

        if(temp > 80){
            over80 = 1;
            over60 = 1;
        } else if(temp > 60){
            over60 = 0;
            over80 = 0;
        } else {
            over60 = 0;
            over80 = 0;
        }

        if(over80){
            target = 0;
        } else if(over60){
            target = 170;
        } else {
            target = 255;
        }

        taskENTER_CRITICAL();
        target_speed = target;
        taskEXIT_CRITICAL();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vPWMTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        uint8_t current, target;

        taskENTER_CRITICAL();
        current = current_speed;
        target = target_speed;
        taskEXIT_CRITICAL();

        if(current < target){
            current += 3;
            if(current > target){
                current = target;
            }
        }
        else if(current > target){
            current -= 3;
            if(current < target){
                current = target;
            }
        }
        PWM_speed(current);

        taskENTER_CRITICAL();
        current_speed = current;
        taskEXIT_CRITICAL();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vButtonTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        if(!(PINA & (1 << PA1))){
            vTaskDelay(pdMS_TO_TICKS(40));
            if(!(PINA & (1 << PA1))){
                PORTB |= (1 << PB0);
                PORTB &= ~(1 << PB1);
                while(!(PINA & (1 << PA1))){
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
        
        if(!(PINA & (1 << PA2))){
            vTaskDelay(pdMS_TO_TICKS(40));
            if(!(PINA & (1 << PA2))){
                PORTB |= (1 << PB1);
                PORTB &= ~(1 << PB0);
                while(!(PINA & (1 << PA2))){
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }

        if(!(PINA & (1 << PA3))){
            vTaskDelay(pdMS_TO_TICKS(40));
            if(!(PINA & (1 << PA3))){
                PORTB &= ~((1 << PB0) | (1 << PB1));
                while(!(PINA & (1 << PA3))){
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vLCDTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        uint16_t temp;
        uint8_t speed;

        taskENTER_CRITICAL();
        temp = temperature;
        speed = current_speed;
        taskEXIT_CRITICAL();

        LCD_cmd(0x01);
        _delay_ms(2);
        LCD_goto(0, 0);
        LCD_string("Temperature: ");
        LCD_integer(temp);
        LCD_string(" C  ");

        LCD_goto(1, 0);
        LCD_string("Speed: ");
        LCD_integer(speed);
        LCD_string("    ");

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vUARTTask(void *pvParameters){
    (void) pvParameters;

    for(;;){
        uint16_t temp;
        uint8_t speed;
        
        taskENTER_CRITICAL();
        temp = temperature;
        speed = current_speed;
        taskEXIT_CRITICAL();

        UART_sendString("Temperature: ");
        UART_sendINT(temp);
        UART_sendString("C,  Speed: ");
        UART_sendINT(speed);
        UART_sendString("\r\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void){
    PWM_init();
    ADC_init();
    UART_init();
    LCD_init();

    DDRB |= (1 << PB0) | (1 << PB1);

    DDRA &= ~((1 << PA1) | (1 << PA2) | (1 << PA3));
    PORTA |= (1 << PA1) | (1 << PA2) | (1 << PA3);

    xTaskCreate(vSensorTask, "SENS", 128, NULL, 2, NULL);
    xTaskCreate(vControlTask, "CTRL", 128, NULL, 2, NULL);
    xTaskCreate(vPWMTask, "PWM", 128, NULL, 3, NULL);
    xTaskCreate(vButtonTask, "BTNS", 128, NULL, 2, NULL);
    xTaskCreate(vLCDTask, "LCD", 256, NULL, 1, NULL);
    xTaskCreate(vUARTTask, "UART", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);

}