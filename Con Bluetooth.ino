//librerias 
#include <SoftwareSerial.h> 
#include <Servo.h> 

//variables no declarativas para los pines 
//se va a usar un solo motor 
#define LED_Y 2 
#define LED_R 3 
#define INT_1 A0 
#define INT_2 A1 

//motor 
#define EA 5
#define EB 4
#define EN 6
#define BUZZ 7 

//sensor proximidad 
#define TRIG 8 
#define ECHO 9 
#define SERVO 11 

//receptor bluetooth 
#define TXD 12 
#define RXD 13 

Servo servo1; 

//asigno los puertos al receptor bluetooth 
SoftwareSerial BTSerial(TXD, RXD); 

//declaracion de variables para el codigo 
// defino los angulos del servo motor, poe ahi hay que cambiarlos 
int centro = 90; 
int izq = 45; 
int der = 135; 
int velocidad = 200; 
long intervalo = 330; // Tiempo en milisegundos 

char dato; 
bool balizaActiva = false; 
long previoMillis = 0; 
bool estadoLuces = LOW; 

void setup() { 
  pinMode(EA, OUTPUT); 
  pinMode(EB, OUTPUT); 
  pinMode(EN, OUTPUT); 
  
  pinMode(BUZZ, OUTPUT); 
  pinMode(TRIG, OUTPUT); 
  pinMode(ECHO, INPUT); 
  
  pinMode(LED_Y, OUTPUT); 
  pinMode(LED_R, OUTPUT); 
  pinMode(INT_1, OUTPUT); 
  pinMode(INT_2, OUTPUT); 
  
  servo1.attach(11); 
  servo1.write(centro); 
  
  Serial.begin(9600); //inicia emulacion serial 
  BTSerial.begin(9600); 
} 

void loop() { 
  //chequea si el bluetooth esta conectado 
  if (BTSerial.available()) { 
    dato = BTSerial.read(); 
    
    //uso comillas porque yo quiero representar el numero ingresado con el teclado que en codigo ascii no es el ismo que el numero matematico 
    if (dato >= '0' && dato <= '9') { 
      //convierte la velocidad de 0-9 a 0-232 para que la use el motor, la velocidad 255 esta asignada a la q 
      //si no gunciona poner el 0 y el 9 entre comillas simples para comvertirlo en codigo ascii 
      velocidad = map(dato, '0', '9', 0, 232); 
      // sobreescribe inmediatamente la velocidad 
      analogWrite(EN, velocidad); 
    } else if (dato == 'q') { 
      velocidad = 255; 
      analogWrite(EN, velocidad); 
    } else { 
      comandos(dato); 
    } 
  } 
  
  if (balizaActiva) { 
    long actualMillis = millis(); 
    if (actualMillis - previoMillis >= intervalo) { 
      previoMillis = actualMillis; // guarda el ultimo tiempo de parpadeo 
      estadoLuces = !estadoLuces; // invierte el estado como la logica del boton 
      digitalWrite(INT_1, estadoLuces); 
      digitalWrite(INT_2, estadoLuces); 
      
      //sonido buzzer
      if (estadoLuces == HIGH) {
        tone(BUZZ, 988, 30); // B5
      } 
      else {
        tone(BUZZ, 784, 30);  //G5
      }
    } 
  } 
} 






//funciones para el motor 
void avanzar(int vel) { 
  digitalWrite(EA, HIGH); 
  digitalWrite(EB, LOW); 
  analogWrite(EN, vel); 
} 

void reversa(int vel) { 
  digitalWrite(EA, LOW); 
  digitalWrite(EB, HIGH); 
  analogWrite(EN, vel); 
} 

void detener() { 
  digitalWrite(EA, LOW); 
  digitalWrite(EB, LOW); 
  analogWrite(EN, 0); 
} 




//switch para los datos ingresados 
void comandos(char com) { 
  switch (com) { 
    case 'F': //avanzar recto 
    servo1.write(centro); 
    avanzar(velocidad); 
    break; 
    
    case 'B': //marchatras recto 
    servo1.write(centro); 
    reversa(velocidad); 
    break; 
    
    // giros de servo sin avanzar 
    case 'L': // girar a la izquierda 
    detener();
    servo1.write(izq); 
    break; 
    
    case 'R': // girar a la derecha 
    detener();
    servo1.write(der); 
    break; 
    
    // Giros y avanze 
    case 'G': // avanzar a la izquierda 
    servo1.write(izq); 
    avanzar(velocidad); 
    break; 
    
    case 'I': // avanzar a la derecha 
    servo1.write(der); 
    avanzar(velocidad); 
    break; 
    
    // giros y retroceso 
    case 'H': // marchatras a la izquierda 
    servo1.write(izq); 
    reversa(velocidad); 
    break; 
    
    case 'J': // marchatras a la derecha 
    servo1.write(der); 
    reversa(velocidad); 
    break; 
    
    // parar, no hacer nada 
    case 'S': 
    detener(); 
    servo1.write(centro); 
    break; 
    
    // Bocina 
    case 'V': 
    tone(BUZZ, 400); // frecuencia de 300 a 500 es la de una bocina 
    break; 
    
    case 'v': 
    noTone(BUZZ); 
    break; 
    
    //luces frontales y traseras fijas 
    case 'W': 
    digitalWrite(LED_Y, HIGH); 
    break; 
    
    case 'w': 
    digitalWrite(LED_Y, LOW); 
    break; 
    
    case 'U': 
    digitalWrite(LED_R, HIGH); 
    break; 
    
    case 'u': 
    digitalWrite(LED_R, LOW); 
    break; 
    

    //para programar la baliza no se podria usar simplemente un deay porque pararia todo el codigo 
    // se usaria millis que cuenta el tiempo desde la placa se prende 
    case 'X': 
    balizaActiva = true; 
    break; 
    
    case 'x': 
    balizaActiva = false; 
    noTone(BUZZ);
    digitalWrite(INT_1, LOW); 
    digitalWrite(INT_2, LOW); 
    break; 
    
    default: 
    break; 
  } 
}
