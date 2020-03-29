
/* MODULO1. ESP32 DevKIT.
 * Version 1
 * 
 * En este modulo se va a controlar el motor que abre y cierra la puerta de acceso.
 * El motor usado es el 28BYJ48 a traves de un controlador ULN2003.
 * 
 * El movimiento del motor se va hacer en modo no bloqueante para pueda el ESP32 hacer otras tareas en los tiempos muertos.
 * Para esto se usará una clase fabricada en una librería para hacer esta tarea.
 * 
 * Conexionado:
 *       ESP32 Dev Module
 *                               *| 3.3V           GND |*
 *                               *| EN              23 |*
 *                               *| VP-36       SCL/22 |*
 *                               *| VN-39            1 |*
 *                               *| 34               3 |* 
 *                               *| 35          SDA/21 |*
 *                               *| 32             GND |*
 *              PULSADOR LUZ ----*| 33              19 |*---- FINAL CARRERA (PullDown)
 *                               *| 25              18 |*---- PULSADOR APERTURA/CIERRE PUERTA
 *                               *| 26               5 |*
 *                      |IN4 ----*| 27              17 |*---- BLUE |
 *                      |IN3 ----*| 14              16 |*----GREEN | LED RGB
 *              ULN2003 |IN2 ----*| 12               4 |*----  RED |
 *                      |        *| GND              0 |*
 *                      |IN1 ----*| 13               2 |*
 *                               *| 9               15 |*
 *                               *| 10               8 |*
 *                               *| 11               7 |*
 *                               *| 5V               6 |*
*/

#include <EspMQTTClient.h>
#include "Stepper28BYJ48.h"
#include "LampRGB.h"


#define DEBUG true
#define PASOS_PUERTA 1650 // Pasos para abrir o cerrar la puerta

#define PIN_FINAL_CARRERA 19
#define PIN_PULSADOR_PUERTA 18
#define PIN_PULSADOR_LAMPARA 33

// Estados Puerta
#define CERRADO 0
#define ABIERTO 1

// Estados Lampara
#define OFF 0
#define ON 1
#define ALARM 2

// Estados del Pulsador
#define READ 0  // Leyendo pulsador
#define PAUSE 1  // Pausa para cambio de estado del interruptor
#define TOGGLE 2   
#define ALARMSHOT 3  // Disparo de Alarma
#define ALARMBLOQ 4  // Bloqueado por disparo de alarma
#define RESET 5 


EspMQTTClient mqttClient(
  "RPi_PA",
  "raspberry",
  "192.168.4.1",  // MQTT Broker server ip
  //"MQTTUsername",   // Can be omitted if not needed
  //"MQTTPassword",   // Can be omitted if not needed
  "modulo1",        // mqttClient name that uniquely identify your device
  1883                // The MQTT port, default to 1883. this line can be omitted
);


// Motor Puerta
Stepper28BYJ48 MotorPuerta(13,12,14,27);  // IN1,IN2,IN3,IN4
int estadoFinalCarrera = CERRADO;

// Lampara y Pulsador
LampRGB LamparaGaraje(4,16,17); // Red, Green, Blue

int statusPushButton = RESET;
unsigned long lastPushTime =0;
const int alarmShotDelay = 3000;


void setup() {
    Serial.begin(115200);
    MotorPuerta.begin();
    LamparaGaraje.begin();
  
    pinMode(PIN_FINAL_CARRERA, INPUT);
    pinMode(PIN_PULSADOR_PUERTA, INPUT);
    pinMode(PIN_PULSADOR_LAMPARA, INPUT);
  
    // Control MQTT
    void  enableDebuggingMessages ( const  bool enabled = !DEBUG );
}


// --------------------------------
//     FUNCIONES MQTT
//---------------------------------

void mySubscribeCallback_puerta(const String &msg) {
    #if DEBUG
        Serial.print("domobox/puerta >> "); Serial.println(msg);
    #endif
    
    if (msg == "cerrar"){
        MotorPuerta.ClockwiseTurn(PASOS_PUERTA);
        return;
    }

    if (msg == "abrir"){
        MotorPuerta.AnticlockwiseTurn(PASOS_PUERTA);
        return;
    }

    if (msg == "estado?"){
        if (estadoFinalCarrera == CERRADO) mqttClient.publish("domobox/puerta", "estado? cerrado" );
        else  mqttClient.publish("domobox/puerta", "estado? abierto" );
        return;
    }

    if (msg.substring(0,5) == "mover"){  //mover x y -x pasos
        String sSteps = msg.substring(6);
        int iSteps = sSteps.toInt();
        Serial.print("mover "); Serial.println(iSteps);
        MotorPuerta.Turn( iSteps);
        return;
    }
    
    #if DEBUG
        Serial.println("domobox/puerta >> no reconocido");
    #endif
}

void mySubscribeCallback_iluminacion_garaje(const String &msg) {
    #if DEBUG
        Serial.print("domobox/iluminacion/garaje >> "); Serial.println(msg);
    #endif
    
    if (msg == "on") {
        LamparaGaraje.on();
        return;
    }
    if (msg == "off") {
        LamparaGaraje.off();
        return;
    }
    if (msg == "conmuta") {
        LamparaGaraje.toggle();
        return;
    }
}

void mySubscribeCallback_alarma(const String &msg) {
    #if DEBUG
        Serial.print("domobox/alarma >> "); Serial.println(msg);
    #endif
    if (msg == "on") {
        LamparaGaraje.alarm();
        return;
    }
    if (msg == "off") {
        statusPushButton = RESET;
        return;
    }
}

// Funcion que se ejecutara al conectar  (Wifi and MQTT)
void onConnectionEstablished(){
    #if DEBUG
        Serial.print("Modulo1. Conexión establecida");
    #endif
    mqttClient.subscribe("domobox/puerta", mySubscribeCallback_puerta);
    mqttClient.subscribe("domobox/iluminacion/garaje", mySubscribeCallback_iluminacion_garaje);
    mqttClient.subscribe("domobox/alarma", mySubscribeCallback_alarma);
}


//-----------------------------------

void loop() {
  int finalCarrera;
  int pulsadorPuerta;

  mqttClient.loop();
  
  MotorPuerta.loop();


  finalCarrera = digitalRead(PIN_FINAL_CARRERA);
  // Cambios de estado en final de carrera (pulsador PullDown)
  if ((finalCarrera == HIGH) && (estadoFinalCarrera == ABIERTO)){  
      mqttClient.publish("domobox/puerta", "estado? cerrado" );
      estadoFinalCarrera = CERRADO;
  }
  if ((finalCarrera == LOW) && (estadoFinalCarrera == CERRADO)){
      mqttClient.publish("domobox/puerta", "estado? abierto" );
      estadoFinalCarrera = ABIERTO;
  }

  // Apertura o cierre con pulsador manual
  pulsadorPuerta = digitalRead(PIN_PULSADOR_PUERTA);
  if (pulsadorPuerta == HIGH){ // si pulsamos
      if (estadoFinalCarrera == ABIERTO) MotorPuerta.AnticlockwiseTurn(PASOS_PUERTA);
      else MotorPuerta.ClockwiseTurn(PASOS_PUERTA);
  }

  LoopButtonLamp();

}


// PULSADOR LOOP.
// Lectura continua de estado del pulsador de encendido
void LoopButtonLamp(){
  int readButton;
  int statusLamp;

  LamparaGaraje.loop();
  
  switch (statusPushButton){
      case READ:
          readButton = digitalRead(PIN_PULSADOR_LAMPARA);
          if (readButton == HIGH) {
              lastPushTime = millis();
              statusPushButton = TOGGLE;
          }
          break;
      case PAUSE: // esperando tiempo de retardo
          readButton = digitalRead(PIN_PULSADOR_LAMPARA);
          if (readButton == LOW){
              lastPushTime = 0; // dejamos de medir el retardo para disparo
              statusPushButton =  READ;
          }
          if ((lastPushTime != 0) && ((millis() - lastPushTime) > alarmShotDelay)) statusPushButton = ALARMSHOT;
          break;
      case TOGGLE:
          statusLamp = LamparaGaraje.toggle();  // solo puede tomar valores ON/OFF
          if (statusLamp == ON) mqttClient.publish("domobox/iluminacion/garaje", "on", 1 );
          else mqttClient.publish("domobox/iluminacion/garaje", "off", 1 );
          statusPushButton = PAUSE;
          break;
      case ALARMSHOT:
          mqttClient.publish("domobox/alarma", "on" );
          LamparaGaraje.alarm(); 
          statusPushButton = ALARMBLOQ;
          break;
      case ALARMBLOQ: // Bloqueado por alarma. Permanece en este estado hasta recibir RESET
          break;
      case RESET:
          LamparaGaraje.off();
          lastPushTime == 0; // dejamos de medir el retardo para disparo
          statusPushButton = READ;
          break;
  }
}
