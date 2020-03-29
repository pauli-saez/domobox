/*  test_button
 *   
 *  Programa para testear el funcionamiento de un pulsador.
 *  La programación del pulsador se ha hecho como maquina de estados finitos.
 *  Los estados son: ON, OFF y disparo temporizado.
 */


// Estado de Lampara
#define READ 0
#define PAUSE 1
#define OFF 2
#define ON 3
#define TIMERSHOT 4

#define PULSADOR 33

const int timerShot = 3000;
unsigned long lastTimer = 0; // temporizador de disparo
int lastStat = OFF;
int stat =0;

void setup() {
  Serial.begin(115200);
  pinMode(PULSADOR, INPUT);
}

void loop() {
    int readPin;
    
    switch (stat){
      case READ:
        readPin = digitalRead(PULSADOR);
        if (readPin == HIGH){
          lastTimer = millis();
          if (lastStat == ON) stat = OFF;
          else stat = ON;
        }
        break;
      case PAUSE:
        readPin = digitalRead(PULSADOR);
        if (readPin == LOW){
          lastTimer =0; // ponemos a 0 para dejar de medir retardo para disparo
          stat = READ;
        }
        if ((lastTimer != 0) && ((millis() - lastTimer) > timerShot)) stat = TIMERSHOT;
        break;
      case OFF:
        Serial.println("Apaga Dispositivo");
        lastStat = OFF;
        stat = PAUSE;
        break;
      case ON:
        Serial.println("Enciende Dispositivo");
        lastStat = ON;
        stat = PAUSE;
        break;
      case TIMERSHOT:
        lastTimer =0;
        Serial.println("Disparo Temporizado");
        stat = PAUSE;
        break;
    }

}
