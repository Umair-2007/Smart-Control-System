# Author
Umair Wanware

# Skills
Platform IO, VS Code, Embedded C, RTOS, PWM, ADC, UART, LCD, Timers, Prescalers

# Features
## Temperature Monitoring
  . Thermal Prorection
  
   .	Reads anlaogue temperature sensor (LM35) using ADC
   
   .	Converts ADC values to real world temperture
  
## Motor Speed Control
  . Fast PWM using Timer0
  
	.	Smooth speed ramp-up and ramp-down
	
	.	Automatic speed reduction at high temperatures
	
## Safety Logic
  . 60°C → Reduced speed
  
  . 80°C → Motor shutdown (thermal protection)
  
## LCD Interfare
  . Displays real time motor temperature
  
  . Displays motor current speed
  
## Button Controls
  . Direction control (Forward/Reverse/Stop)
  
  . Soft deboumcing using TIMER1
  
## UART communication
  . Sends tempearture data to PC (9600 baud)
  
  . Ready for GUI or logging integration
  
## Precise Timing
  . 10ms delay using TIMER1 (CTC mode)
