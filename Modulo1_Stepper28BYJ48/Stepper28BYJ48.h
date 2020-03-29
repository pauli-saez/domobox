/* StepperBYJ48.h
 * 
 * Clase para control de motor 28BYJ48
 * 
 * By paulino.saezruiz@gmail.com
 */


class Stepper28BYJ48
{
  private:

    const int _stepsFullTurn = 4076;  // Pasos para una vuelta completa
    const int _stepsHalfTurn = 2038;  // Pasos para media vuelta

    //tablas con la secuencia de encendido (descomentar la que necesiteis)
    //secuencia 1-fase
    //const int _numSteps = 4;
    //const int _stepsLookup[4] = { B1000, B0100, B0010, B0001 };
     
    //secuencia 2-fases
    //const int _numSteps = 4;
    //const int _stepsLookup[4] = { B1100, B0110, B0011, B1001 };

    //secuencia media fase (recomendada por el fabricante)
    const int _numSteps = 8;
    const int _stepsLookup[8] = { B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001 };
    int _step =0;  // contador que nos va indicar la posición stepsLookup a leer
    

    int _in1; int _in2; int _in3; int _in4;  // Pines a bobinas de motor
    int _motorSpeed =1200; // tiempo entre impulsos (1000 = 10 milisegundos)
     
    // Estados
    float _statusStep = 0;  // contador de pasos totales. Si 0 es posición de origen.
    unsigned long _timer = 0; // Contador para la temporización de los pasos
    int _desplacement = 0; // sera un contador hasta 0 para fijar el desplazamiento

  public:

    //Constructor
    Stepper28BYJ48(int in1,int in2, int in3, int in4){  // pines usados en el controlador ULN2003 
        _in1 = in1;
        _in2 = in2;
        _in3 = in3;
        _in4 = in4;
      }

    void begin(){
        //declarar pines como salida
        pinMode(_in1, OUTPUT);
        pinMode(_in2, OUTPUT);
        pinMode(_in3, OUTPUT);
        pinMode(_in4, OUTPUT);
    }
    
    //Lectura continua de estado de motor
    void loop(){
      if (_desplacement == 0) return;
      if (_timer + _motorSpeed < micros()){
        _timer = micros();
        if (_desplacement > 0){
          stepClockwise();
          _desplacement--;
          _statusStep++;
        }
        else{
          stepAnticlockwise();
          _desplacement++;
          _statusStep--;
        }
      }
      return;
    }
    

    // Prepara para que comience con los giros en sentido horario y antihorario
    void Turn(int steps){  // sentido horario +steps, sentido antihorario -steps
      if (_desplacement == 0) { // no ejecuta un movimiento si ya hay uno en ejecución
        _desplacement = steps;
        _timer = micros();
      }
    }

    void OriginTurn(){ Turn(_statusStep*(-1)); }
    void ClockwiseTurn(int steps) { Turn(steps); }
    void AnticlockwiseTurn( int steps) { Turn(steps*(-1)); }

    String  getInfo() {
      String trama;
      trama = "Desp:"; trama += String(_desplacement); trama += " . ";
      trama += "SS:"; trama += String(_statusStep); trama += " . ";
      trama += "Bobina:"; trama += String(_step);
      return ( trama);
    }


    // movimiento del motor paso a paso en sentido horario y antihorario
    void stepClockwise(){
      _step++;
      if (_step >= _numSteps) _step = 0; 
      setOutput(_step);
    }
     
    void stepAnticlockwise(){
      _step--;
      if (_step < 0) _step = _numSteps-1;
      setOutput(_step);
    }

    void setOutput(int step){
      digitalWrite(_in1, bitRead(_stepsLookup[step], 0));
      digitalWrite(_in2, bitRead(_stepsLookup[step], 1));
      digitalWrite(_in3, bitRead(_stepsLookup[step], 2));
      digitalWrite(_in4, bitRead(_stepsLookup[step], 3));
    }
    
};
