/* LampRGB.h 
 *  Clase para el control de la iluminación de habitaciones.
 *  Los estados definidos pueden ser luz encendida, apagada o disparada alarma (led rojo parapdeando)
*/

// Estado de Lampara
#define OFF 0
#define ON 1
#define ALARM 2


class LampRGB
{
  private:

    int _pin_red; int _pin_green; int _pin_blue;
    
    int _blink_timer = 500; // Velocidad de parpadeo
    unsigned long _timer = 0; // Contador para encendido apagado
    bool _redOn = false;

    int _statusLamp; 

  public:
    
    //Constructor
    LampRGB( int pin_red, int pin_green, int pin_blue ){
      _pin_red = pin_red;
      _pin_green = pin_green;
      _pin_blue = pin_blue;
    }

   void begin(){
        //declarar pines como salida
        pinMode(_pin_red, OUTPUT);
        pinMode(_pin_green, OUTPUT);
        pinMode(_pin_blue, OUTPUT);
        _statusLamp = OFF; // Estado inicial de la lampara (0 Off, 1 On, 2 Alarm)
    }

    
    //Lectura continua de estado del led
    void loop(){
      if (_statusLamp < ALARM) return;
      if ((_timer + _blink_timer) < millis()){
        SW_Red();
      }
      return;
    }

    void on(){  // Luz blanca
      digitalWrite(_pin_red,HIGH);
      digitalWrite(_pin_green,HIGH);
      digitalWrite(_pin_blue,HIGH);
      _statusLamp = ON;
    }

    void off(){  // Apagar Luz
      digitalWrite(_pin_red,LOW);
      digitalWrite(_pin_green,LOW);
      digitalWrite(_pin_blue,LOW);
      _statusLamp = OFF;
    }

    void toggle(){  // Apagar/Encender Luz
      if (_statusLamp == ON) off();
      else if (_statusLamp == OFF) on();
      return;
    }
    
    void alarm(){ // Parpadero continuo led Rojo
      digitalWrite(_pin_green, LOW);
      digitalWrite(_pin_blue, LOW);
      _statusLamp = ALARM;
      _redOn = false;;
      SW_Red();
      
    }

    void red(){
      digitalWrite(_pin_red, HIGH);
      digitalWrite(_pin_green, LOW);
      digitalWrite(_pin_blue, LOW);
      _statusLamp = ON;
    }

    void green(){
      digitalWrite(_pin_red, LOW);
      digitalWrite(_pin_green, HIGH);
      digitalWrite(_pin_blue, LOW);
      _statusLamp = ON;
    }

    void blue(){
      digitalWrite(_pin_red, LOW);
      digitalWrite(_pin_green, LOW);
      digitalWrite(_pin_blue, HIGH);
      _statusLamp = ON;
    }
    
    void SW_Red(){  // Cambia rojo de encendido a apagado para parpadeo
      if (_redOn){
        digitalWrite(_pin_red, LOW);
        _redOn = false;
      } 
      else {
        digitalWrite(_pin_red, HIGH);
        _redOn = true;
      }
      _timer = millis();
    }
};
