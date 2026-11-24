#include <SoftwareSerial.h>
#include <Servo.h>
// #include <Wire.h>                     // <--- DESACTIVADO TEMPORALMENTE
// #include <Adafruit_VL53L1X.h>         // <--- DESACTIVADO TEMPORALMENTE
// #include <DFRobotDFPlayerMini.h>      // <--- DESACTIVADO TEMPORALMENTE

// Receptor de radio
#define PIN_DIR 2
#define PIN_GAS 3

// Salidas (pines 9 y 10 como tu prueba)
#define SERVO_PIN 9
#define ESC_PIN 10
#define BUZZ 7
#define LED_FRENTE 6
#define LED_ATRAS 5
#define GIRO_IZQ A0
#define GIRO_DER A1

// Bluetooth
#define TXD 12
#define RXD 13
SoftwareSerial BTSerial(TXD, RXD);

Servo esc;
Servo direccion;
// Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(); // <--- DESACTIVADO TEMPORALMENTE

#define NEUTRO 1515
#define DIR_MIN_PULSO 1148
#define DIR_MAX_PULSO 2100
#define ZONA_MUERTA 40

#define DIST_FRENO 30
#define DIST_LIBRE 40
#define GIRO_MIN 35

bool maxPotencia = false;
bool antiChoque = false;
bool balizaActiva = false;
bool estadoBaliza = false;
bool faroActivo = false;
bool bloqueado = false;
int distancia = 0;
unsigned long ultimaLectura = 0;
unsigned long previoBaliza = 0;
const unsigned long INTERVALO_BALIZA = 330;

char dato;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_DIR, INPUT);
  pinMode(PIN_GAS, INPUT);

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

  esc.attach(ESC_PIN);
  esc.writeMicroseconds(NEUTRO);
  delay(3000);

  // Todo lo del DFPlayer y VL53L1X queda desactivado para que no cuelgue el setup
}

void loop() {
  // leerDistancia(); // <--- DESACTIVADO TEMPORALMENTE

  // Sin timeout largo para probar idéntico a tu código original
  int pulsoDir = pulseIn(PIN_DIR, HIGH, 30000);
  int pulsoGas = pulseIn(PIN_GAS, HIGH, 30000);

  int angulo = 90;
  if (pulsoDir > 0) {
    angulo = map(pulsoDir, DIR_MIN_PULSO, DIR_MAX_PULSO, 180, 0);
    angulo = constrain(angulo, 0, 180);
  }
  direccion.write(angulo);

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

  if (BTSerial.available()) {
    dato = BTSerial.read();
    comandos(dato);
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
    case 'A': antiChoque = true; break;
    case 'a': antiChoque = false; break;
  }
}
