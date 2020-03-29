# I2C Keypad
El Teclado I2C es un módulo que permite conectar un Keypad con PCF8574, que es un IC para expandir los puertos de Entrada/Salida Digital.
Trabaja a través de I2C entre el microcontrolador y el teclado usando pines de E/S digitales, lo que reduce las necesidades de GPIOs 

Está probado con ESP32 LOLIN D32 con un Keypad de tipo boton, funcionando correctamente.

La dirección predeterminada del teclado I2C es 0x20. (A0-, A1-, A2-)

