# Author
Umair Wanware

# Skills
Platform IO, VS Code, Embedded C, RTOS, PWM, ADC, UART, LCD, Timers, Prescalers

## Features

### Temperature Monitoring
- Thermal Protection
- Reads analogue temperature sensor (LM35) using ADC
- Converts ADC values to real world temperature

### Motor Speed Control
- Fast PWM using Timer0
- Smooth speed ramp-up and ramp-down
- Automatic speed reduction at high temperatures

### Safety Logic
- 60°C → Reduced speed
- 80°C → Motor shutdown (thermal protection)

### LCD Interface
- Displays real time motor temperature
- Displays motor current speed

### Button Controls
- Direction control (Forward/Reverse/Stop)
- Soft debouncing using TIMER1

### UART Communication
- Sends temperature data to PC (9600 baud)
- Ready for GUI or logging integration

### Precise Timing
- 10ms delay using TIMER1 (CTC mode)
