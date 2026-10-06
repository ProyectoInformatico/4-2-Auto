//librerias
#include <Servo.h>

//variables no declarativas para los pines
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

Servo servo1;

//declaracion de variables para el codigo
int centro = 90;
int izq = 45;
int der = 135;
char dato;
int velocidad = 200;

bool balizaActiva = false;
long previoMillis = 0;
long intervalo = 330; // tiempo en milisegundos para la baliza
bool estadoLuces = LOW;

void setup() {
  pinMode(EA, OUTPUT);
  pinMode(EB, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  
  // TRIG es OUTPUT y ECHO es INPUT
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(INT_1, OUTPUT);
  pinMode(INT_2, OUTPUT);
  
  servo1.attach(SERVO);
  servo1.write(centro);
  
  // Iniciamos la consola serie a 9600 baudios
  Serial.begin(9600); 
  Serial.println("Consola lista. Ingrese un comando (ej: F, B, S, G, 0-9):");
}


void loop() {
  // Chequea si se escribió algo en el Monitor Serie de la computadora
  if (Serial.available() > 0) {
    dato = Serial.read();
    
    // Ignora los caracteres de salto de línea que envía la consola al presionar Enter
    if (dato == '\n' || dato == '\r') {
      return; 
    }

    // Muestra en la misma consola el comando que recibió para confirmar
    Serial.print("Comando recibido: ");
    Serial.println(dato);
    
    if (dato >= '0' && dato <= '9') {
      velocidad = map(dato, '0', '9', 0, 232);
      analogWrite(EN, velocidad);
      Serial.print("Velocidad ajustada a: ");
      Serial.println(velocidad);
    } else if (dato == 'q') {
      velocidad = 255;
      analogWrite(EN, velocidad);
      Serial.println("Velocidad Máxima (255)");
    } else {
      comandos(dato);
    }
  }

  
  
  // Lógica de balizas usando millis()
  if (balizaActiva) {
    long actualMillis = millis();
    if (actualMillis - previoMillis >= intervalo) {
      previoMillis = actualMillis;
      estadoLuces = !estadoLuces;
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
    case 'F': // Avanzar recto
      servo1.write(centro);
      avanzar(velocidad);
      break;
      
    case 'B': // Marchatras recto
      servo1.write(centro);
      reversa(velocidad);
      break;
      
    // Giros de servo sin avanzar
    case 'L': // Girar a la izquierda
      detener();
      servo1.write(izq);
      break;
    case 'R': // Girar a la derecha
      detener();
      servo1.write(der);
      break;
      
    // Giros y avance
    case 'G': // Avanzar a la izquierda
      servo1.write(izq);
      avanzar(velocidad);
      break;
    case 'I': // Avanzar a la derecha
      servo1.write(der);
      avanzar(velocidad);
      break;
      
    // Giros y retroceso
    case 'H': // Marchatras a la izquierda
      servo1.write(izq);
      reversa(velocidad);
      break;
    case 'J': // Marchatras a la derecha
      servo1.write(der);
      reversa(velocidad);
      break;
      
    // Parar, no hacer nada
    case 'S':
      detener();
      servo1.write(centro);
      break;
      
    // Bocina
    case 'V':
      tone(BUZZ, 400);
      break;
    case 'v':
      noTone(BUZZ);
      break;
      
    // Luces frontales y traseras fijas
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
      
    // Intermitentes (balizas)
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
      Serial.println("Comando no reconocido.");
      break;
  }
}
