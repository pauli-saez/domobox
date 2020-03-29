/* Test_buttonAndLedRGB
 * Pruebas ButtonAndLampRGB
 * Pruebas para crear una libreria funcional LampRGB
 * El sistema utiliza un boton para encender y apagar el led en blanco, pero si 
 * mantenemos pulsado el boton durante 3 segundos de enciende intermitentemente
 * el led en rojo
 */

#include "LampRGB.h"

// Estados del Pulsador
#define READ 0  // Leyendo pulsador
#define PAUSE 1  // Pausa para cambio de estado del interruptor
#define TOGGLE 2   
#define ALARMSHOT 3  // Disparo de Alarma
#define ALARMBLOQ 4  // Bloqueado por disparo de alarma
#define RESET 5 


const int red = 4;
const int green = 16;
const int blue = 17;
const int button = 19;

LampRGB lampara(red, green, blue);

int statusPushButton;
unsigned long lastPushTime =0;
const int alarmShotDelay = 3000;

 
void setup() {
    Serial.begin(115200);
    lampara.begin();
    pinMode(button, INPUT);

    statusPushButton = RESET;
}

void loop() {
    LoopButtonLamp();
    lampara.loop();
}


// ---- PULSADOR
//Lectura continua de estado del pulsador de encendido
void LoopButtonLamp(){
  int readButton;
  
  switch (statusPushButton){
      case READ:
          readButton = digitalRead(button);
          if (readButton == HIGH) {
              lastPushTime = millis();
              statusPushButton = TOGGLE;
          }
          break;
      case PAUSE: // esperando tiempo de retardo
          readButton = digitalRead(button);
          if (readButton == LOW){
              lastPushTime = 0; // dejamos de medir el retardo para disparo
              statusPushButton =  READ;
          }
          if ((lastPushTime != 0) && ((millis() - lastPushTime) > alarmShotDelay)) statusPushButton = ALARMSHOT;
          break;
      case TOGGLE:
          lampara.toggle();
          statusPushButton = PAUSE;
          break;
      case ALARMSHOT:
          lampara.alarm(); 
          statusPushButton = ALARMBLOQ;
          break;
      case ALARMBLOQ: // Bloqueado por alarma. Solo sale de este estado con un reset()
          Serial.println("Estado ALARMBLOQ");
          break;
      case RESET:
          lampara.off();
          lastPushTime == 0; // dejamos de medir el retardo para disparo
          statusPushButton = READ;
          break;
  }
}
