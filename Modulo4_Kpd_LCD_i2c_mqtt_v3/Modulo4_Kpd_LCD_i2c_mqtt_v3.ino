/*
 * MODULO4. LOLIN D32.
 * Version 3.1
 * 
 * Este modulo va a controlar mediante keypad y display LCD, conectado con bus I2C, el acceso a de entrada con password de Domobox.
 * Se conecta un sensor de ultrasonidos para controlar el acercamiento a la entrada
 * 
 * Informacion:
 * El display estará apagado hasta que pulsemos #.
 * El control de acceso ade mas de la clave acepta tambien algunos comandos entre ellos cambio de clave.
 * La clave esta guarda en la memoria eeprom.
 * Para validar la clave o comando se pulsa # que hace las veces de <enter key>
 * 
 * Conexionado:
 *   LOLIN D32.
 *                          *| 3.3V        GND |*
 *                          *| RST           1 |*
 *                          *| VP            3 |*
 *                          *| VN         3.3V |*
 *    HC-SRC04 | Trigger -- *| 32           22 |* ---- SCL |  KEYPAD & LCD
 *             | Echoe -----*| 33           21 |* ---- SDA |
 *                          *| 34          GND |*
 *                          *| 35          GND |*
 *                          *| 25           19 |*
 *                          *| 26           23 |*
 *                          *| 27           18 |*
 *                          *| 14            5 |*
 *                          *| 12         3.3V |*
 *                          *| 13           17 |* 
 *                          *| 5V           16 |*
 *                          *| GND           4 |*
 *                           |               0 |*
 *                           |             GND |*
 *                           |               2 |*
 *                           |              15 |*
*/


#include "EspMQTTClient.h"
#include <Ultrasonic.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <i2c_keypad.h>
#include <Wire.h> 

// Parametros
#define TIMER_SONAR 400 // Espacio entre cada medida de ultrasonido
#define EEPROM_SIZE 10 // define the number of bytes you want to access
#define TIMER_BYE 15000 // 15seg
#define DEBUG true


// Estados
#define STATUS_OFF 0 //Dispay Apagado
#define STATUS_IN 1  //Bienvenida para intoducir datos
#define STATUS_BLOQ 2  //Sistema bloqueado para acceder
#define STATUS_COM1_PASS 3  //Comando 1. *C1** Modificar password
#define STATUS_COM1_NEWPASS 4 //Comando 1. *C1** Clave correcta y en espera de nueva clave
#define STATUS_COM1_GRABA 5  //Comando 1. Grabada nueva clave
#define STATUS_COM2 6 //Comando 2. informar de password actual
#define STATUS_ERROR 7 //Clave equivocada
#define STATUS_OK  8 // Clave correcta
#define STATUS_MAX_DIG 9 // Superado el maximo de digitos


EspMQTTClient client(
  "RPi_PA",
  "raspberry",
  "192.168.4.1",  // MQTT Broker server ip
  //"MQTTUsername",   // Can be omitted if not needed
  //"MQTTPassword",   // Can be omitted if not needed
  "modulo4",        // Client name that uniquely identify your device
  1883                // The MQTT port, default to 1883. this line can be omitted
);

// Defines pins numbers  for Sonar HC-SR04)
Ultrasonic ultrasonic(32, 33); // (Trig, Echoe)

I2CKEYPAD kpd(21, 22);  // sda y sdl en ESP32 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


uint8_t estado=0;
bool enter = false; //indicara si se pulsó #

char password[6]; // password del sistema
char cadena[6]=""; // acumula teclas pulsadas
int position = 0; // position del digito a introducir

unsigned long t_KeypadON =0; // timer de inicio keypad
unsigned long t_SonarON =0; // timer de inicio Ultrasonidos

void setup() {
    Serial.begin(115200);
    lcd.begin();
    lcd.noBacklight();       
    kpd.begin(0x20, 100);

    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Fallo al inicializar EEPROM");
        Serial.println("Restarting...");
        delay(1000);
        ESP.restart();
    }

    EEPROM.get(0,password);
    #if DEBUG
      Serial.print("Clave Actual: "); Serial.print(password);Serial.println(".");
    #endif

    kpd.on(PRESS, [](char kpd) {   // void on(KEYPAD_EVENT event, KeypadEventCallback callback) ;
        t_KeypadON = millis(); // Cada vez que pulsamos una tecla reinicia el timer del keypad
        
        if (kpd == '#') enter = true; 
        else if (position == 5) estado = STATUS_MAX_DIG;
        else {
              cadena[position] = kpd;
              cadena[position+1] = '\0';
              lcd.setCursor( 4 + position, 1);
              lcd.print("*");
              position++;
        }

        #if DEBUG
          Serial.print("PRESS: "); Serial.print(kpd);
          Serial.print(" ... Passw : "); Serial.print(password);Serial.print(".");
          Serial.print(" ... estado/posicion/cadena : ");
          Serial.print(estado); Serial.print("/"); Serial.print(position); Serial.print("/"); Serial.print(cadena);Serial.println(".");
        #endif
    });

    // Control MQTT
    void  enableDebuggingMessages ( const  bool enabled = !DEBUG );

}


// --------------------------------
//     CAMBIOS DE ESTADO
//---------------------------------

// Paso a estado bloqueado
void status2BLOQ(){
    t_KeypadON =0;
    strcpy(cadena, "");
    position =0;
    lcd.noBacklight();
    estado = STATUS_BLOQ;
    #if DEBUG
      Serial.println("STATUS: BLOQ");
    #endif
}

// Pasa a estado OFF
void status2OFF(){
    client.publish("domobox/lcdkeypad", "off"); //mensaje MQTT
    t_KeypadON =0;
    strcpy(cadena,"");
    position =0;
    lcd.clear();
    lcd.noBacklight();
    estado = STATUS_OFF;
    #if DEBUG
      Serial.println("STATUS: OFF");
    #endif
}

// Error de clave
void status2ERROR(){
    client.publish("domobox/lcdkeypad", "password_error"); //mensaje MQTT
    strcpy(cadena,"");
    position =0;
    lcd.clear();lcd.setCursor(0, 0); lcd.print("*** ERROR ***");
    t_KeypadON = millis();
    estado = STATUS_ERROR;
    #if DEBUG
      Serial.println("STATUS: ERROR");
    #endif
}

// Mensaje de Bienvenida y resetea valores
void status2IN(){
    client.publish("domobox/lcdkeypad", "in"); //mensaje MQTT
    strcpy(cadena,"");
    position =0;
    lcd.backlight();
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Introducir Clave");
    estado = STATUS_IN;
}

// Clave aceptada
void status2OK(){
    client.publish("domobox/lcdkeypad", "password_ok"); //mensaje MQTT
    lcd.clear();lcd.setCursor(0, 0); lcd.print("** ACEPTADA **");
    t_KeypadON = millis();
    estado = STATUS_OK;
    #if DEBUG
      Serial.println("STATUS: OK");
    #endif
    return;
}


// Comando1. Solicita clave actual
void status2COM1_PASS(){
    strcpy(cadena,"");
    position =0;
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("C1.Clave Actual?");
    estado = STATUS_COM1_PASS;
    return;
}

// Comando1. Solicita nueva clave
void status2COM1_NEWPASS(){
    strcpy(cadena,"");
    position =0;
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("C1.Nueva Clave?");
    estado = STATUS_COM1_NEWPASS;
    return;
}

// Comando1. Graba nueva clave
void status2COM1_GRABA(){
    int lenArrayCadena = 5;
    for (int x = 0; x <= lenArrayCadena; x++){
       password[x] = cadena[x];
    }
    EEPROM.put(0,password);
    EEPROM.commit();
    EEPROM.get(0,password);
    
    client.publish("domobox/lcdkeypad", "password_change"); //mensaje MQTT
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("*** CAMBIADA ***");
    t_KeypadON = millis();
    estado = STATUS_COM1_GRABA;
    #if DEBUG
        Serial.print("grabada clave: "); Serial.println(password);
    #endif
    return;
}

// Comando2. Informa clave actual
void status2COM2(){
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("*** ENVIADA ***");
    t_KeypadON = millis();
    estado = STATUS_COM2;
    return;
}


// --------------------------------
//     FUNCIONES MQTT
//---------------------------------

void mySubscribeCallback_lcdkeypad(const String &msg) {
    #if DEBUG
        Serial.print("domobox/lcdkeypad >> "); Serial.println(msg);
    #endif
    
    if (msg == "bloquear"){
        status2BLOQ();
        return;
    }
    if (msg == "desbloquear") {
        status2OFF();
        return;
    }
    if (msg == "clave?"){ 
      client.publish("domobox/lcdkeypad", "clave: " +String(password)); //mensaje MQTT
      return;
    }
    if (msg == "estado?"){ 
      client.publish("domobox/lcdkeypad", "estado: " +String(estado)); //mensaje MQTT
    return;
    }
}

void mySubscribeCallback_ultrasonidos(const String &msg) {
    #if DEBUG
      Serial.print("domobox/ultrasonidos >> "); Serial.println(msg);
    #endif
}

/*
void myDelayCallback() {
    client.publish("domobox/lcdkeypad", "conectado"); //mensaje MQTT
    #if DEBUG
      Serial.println("Conectado a MQTT");
    #endif
}
*/

// Funcion que se ejecutara al conectar  (Wifi and MQTT)
void onConnectionEstablished(){
    client.subscribe("domobox/lcdkeypad", mySubscribeCallback_lcdkeypad);
    client.subscribe("domobox/ultrasonidos", mySubscribeCallback_ultrasonidos);
    // client.executeDelayed(1000, myDelayCallback);
}

//-----------------------------


void loop() {
  float distance;
  unsigned long t_Now = millis();

  // *** bucle MQTT
  client.loop();
  
  // *** Envio Ultrasonidos
  if (t_Now - t_SonarON > TIMER_SONAR) {
    t_SonarON = t_Now;
    distance = ultrasonic.read(CM);
    client.publish("domobox/ultrasonidos", String(distance).c_str());
    #if DEBUG
      Serial.println("Distance: " + String(distance));
    #endif
  }

  // *** lcdkeypad
  if (estado != STATUS_BLOQ){
    t_Now = millis();
    if (t_KeypadON > 0 && t_Now > (t_KeypadON+TIMER_BYE))  status2OFF();
    kpd.scand();
  }

  switch (estado){  
    case STATUS_OFF:
        if (enter){
          enter = false;
          status2IN();
        }
        break;
    case STATUS_OK:
       if (t_Now - t_KeypadON > 3000) status2OFF();
       break;
    case STATUS_ERROR:
       if (t_Now - t_KeypadON > 3000) status2IN();
       break;
    case STATUS_MAX_DIG:
        status2ERROR();
        break;
    case STATUS_IN: // Podemos introducir la clave o un comando preprogramado
        if (enter){
          enter = false;
          if (String(cadena) == String(password)) status2OK();
          else if (String(cadena)== "*C1**") status2COM1_PASS();
          else if (String(cadena)== "*C2**") status2COM2();
          else status2ERROR();
        }
        break;
    case STATUS_COM1_PASS:
        if (enter){
          enter =false;
          if (String(cadena) == String(password)) status2COM1_NEWPASS();
          else status2ERROR();
        }
        break;
    case STATUS_COM1_NEWPASS:
        if (enter){
          enter =false;
          if ((String(cadena).indexOf('*') > -1 ) ||  //Clave no puede contener asteriscos
              (String(cadena) == String(password))) status2ERROR();
          else status2COM1_GRABA();
        }
        break;
    case STATUS_COM1_GRABA:
       if (t_Now - t_KeypadON > 3000) status2OFF();
       break;      
    case STATUS_COM2:
       Serial.print("contador: "); Serial.println(t_Now-t_KeypadON);
       if (t_Now > (t_KeypadON+3000)) status2OFF();
       break; 
  }
}
