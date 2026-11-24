#include <SoftwareSerial.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>

// Receptor de radio
#define PIN_DIR 2
#define PIN_GAS 3

// Salidas
#define SERVO_PIN 9
#define ESC_PIN 10
#define BUZZ 7
#define LED_FRENTE 6         // Luces delanteras / posicion (PWM)
#define LED_ATRAS 5          // Luces traseras / reversa (PWM)
#define GIRO_IZQ A0
#define GIRO_DER A1

// Bluetooth y pin de estado físico
#define BT_STATE 8           // Conectar al pin STATE del modulo Bluetooth
#define TXD 12
#define RXD 13
SoftwareSerial BTSerial(TXD, RXD);

// DFPlayer (MP3)
#define MP3_RX A2
#define MP3_TX A3            // Al RX del DFPlayer con resistencia de 1k
SoftwareSerial mp3Serial(MP3_RX, MP3_TX);
DFRobotDFPlayerMini mp3;

Servo esc;
Servo direccion;

#define NEUTRO 1515
#define DIR_MIN_PULSO 1148
#define DIR_MAX_PULSO 2100
#define ZONA_MUERTA 40
#define GIRO_MIN 35

bool maxPotencia = false;
bool balizaActiva = false;
bool estadoBaliza = false;
bool faroActivo = false;
unsigned long previoBaliza = 0;
const unsigned long INTERVALO_BALIZA = 330;

// Variables para detectar conexion/desconexion fisica del BT
bool btConectado = false;
unsigned long tiempoEnLow = 0;

char dato;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_DIR, INPUT);
  pinMode(PIN_GAS, INPUT);
  pinMode(BT_STATE, INPUT);

  pinMode(BUZZ, OUTPUT);
  pinMode(LED_FRENTE, OUTPUT);
  pinMode(LED_ATRAS, OUTPUT);
  pinMode(GIRO_IZQ, OUTPUT);
  pinMode(GIRO_DER, OUTPUT);

  analogWrite(LED_FRENTE, 64);
  analogWrite(LED_ATRAS, 64);
  digitalWrite(GIRO_IZQ, LOW);
  digitalWrite(GIRO_DER, LOW);

  direccion.attach(SERVO_PIN);
  direccion.write(90);

  BTSerial.begin(9600);

  mp3Serial.begin(9600);
  mp3.begin(mp3Serial, false, false);
  BTSerial.listen();

  esc.attach(ESC_PIN);
  esc.writeMicroseconds(NEUTRO);
  delay(3000);

  // 1) Sonido al prender el Arduino: carpeta 01, archivo 001 (arranque de motor)
  mp3.volume(20);
  mp3.playFolder(1, 1);
}

void loop() {
  // 0) Deteccion automatica de estado del Bluetooth (pin STATE -> D8)
  verificarBluetooth();

  // 1) Receptor de radio
  int pulsoDir = pulseIn(PIN_DIR, HIGH, 30000);
  int pulsoGas = pulseIn(PIN_GAS, HIGH, 30000);

  // 2) Servo de direccion
  int angulo = 90;
  if (pulsoDir > 0) {
    angulo = map(pulsoDir, DIR_MIN_PULSO, DIR_MAX_PULSO, 180, 0);
    angulo = constrain(angulo, 0, 180);
  }
  direccion.write(angulo);

  // 3) Motor
  int gas = NEUTRO;
  if (pulsoGas > 0) gas = pulsoGas;

  bool quiereRetroceder = gas < NEUTRO - ZONA_MUERTA;

  analogWrite(LED_FRENTE, faroActivo ? 255 : 64);
  analogWrite(LED_ATRAS, quiereRetroceder ? 255 : 64);

  if (!maxPotencia) {
    gas = NEUTRO + (gas - NEUTRO) / 2;
  }
  gas = constrain(gas, 1000, 2000);
  esc.writeMicroseconds(gas);

  // 4) Baliza y luces de giro
  unsigned long ahora = millis();
  if (ahora - previoBaliza >= INTERVALO_BALIZA) {
    previoBaliza = ahora;
    estadoBaliza = !estadoBaliza;
    if (balizaActiva) tone(BUZZ, estadoBaliza ? 1000 : 784, 30);
  }

  if (balizaActiva) {
    digitalWrite(GIRO_IZQ, estadoBaliza ? HIGH : LOW);
    digitalWrite(GIRO_DER, estadoBaliza ? HIGH : LOW);
  } else {
    bool giroIzq = angulo <= 90 - GIRO_MIN;
    bool giroDer = angulo >= 90 + GIRO_MIN;
    digitalWrite(GIRO_IZQ, (estadoBaliza && giroIzq) ? HIGH : LOW);
    digitalWrite(GIRO_DER, (estadoBaliza && giroDer) ? HIGH : LOW);
  }

  // 5) Comandos de la app por Bluetooth
  if (BTSerial.available()) {
    dato = BTSerial.read();
    comandos(dato);
  }
}

void verificarBluetooth() {
  int estadoPin = digitalRead(BT_STATE);

  if (estadoPin == HIGH) {
    tiempoEnLow = 0; // Reinicia el contador de desconexion
    if (!btConectado) {
      btConectado = true;
      mp3.playFolder(2, 1); // Carpeta 02, archivo 001 -> Conectado
    }
  } else {
    // Si esta en LOW, esperamos 1.2 segundos continuos para confirmar desconexion
    // (evita falsos disparos si el pin STATE parpadea en algunos modulos)
    if (btConectado) {
      if (tiempoEnLow == 0) {
        tiempoEnLow = millis();
      } else if (millis() - tiempoEnLow > 1200) {
        btConectado = false;
        tiempoEnLow = 0;
        mp3.playFolder(2, 2); // Carpeta 02, archivo 002 -> Desconectado
      }
    }
  }
}

void comandos(char com) {
  switch (com) {
    case 'V': tone(BUZZ, 400); break;
    case 'v': noTone(BUZZ); break;
    case 'W': faroActivo = true; break;
    case 'w': faroActivo = false; break;
    case 'X': balizaActiva = true; break;
    case 'x': balizaActiva = false;
              noTone(BUZZ);
              digitalWrite(GIRO_IZQ, LOW);
              digitalWrite(GIRO_DER, LOW);
              break;
    case 'P': maxPotencia = true; break;
    case 'p': maxPotencia = false; break;
  }
}
