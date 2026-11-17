/*
   ─────────────────────────────────────────────────────────────
   RC EcoSport - Arduino intermediario + sensor laser VL53L1X
   ─────────────────────────────────────────────────────────────
   El receptor de radio maneja el motor y el servo.
   La app Android (RC_Controller_EcoSport) solo manda ajustes:
     BOCINA   : V (mantener) / v (soltar)
     FARO     : W (100%) / w (posicion 25%)
     BALIZA   : X / x
     EcoSport : P / p
     Anti choque : A / a (la app manda el estado al conectar)
     Conectado : C (aviso para el sonido de conectado)
     Desconectar : D (aviso para el sonido de desconectado)

   EcoSport desactivado (por defecto) = mitad de potencia.
   EcoSport activado = potencia completa.

   SENSOR DE DISTANCIA (VL53L1X)
   - Si hay un objeto a menos de 30 cm, el auto frena y queda
     BLOQUEADO: no puede avanzar.
   - Estando bloqueado solo podes: dar marcha atras, o girar
     bastante (35 grados o mas) y avanzar.
   - El bloqueo se libera cuando el sensor vuelve a leer mas de
     40 cm (te alejaste) o cuando estas girando lo suficiente.
   - La funcion arranca APAGADA. Se prende desde la app
     (boton ANTI CHOQUE). Al conectarse, la app manda el estado
     de todos los botones, asi el Arduino queda igual que la
     pantalla. Apagada, el sensor no frena ni bloquea nada.

   SONIDOS (DFPlayer Mini + parlante)
   - La microSD va en FAT32 (hasta 32 GB) con esta estructura:
       01/001.mp3  -> arranque de motor (suena al prender)
       02/001.mp3  -> bluetooth conectado
       02/002.mp3  -> bluetooth desconectado
   - El volumen se cambia en mp3.volume(20) (va de 0 a 30).
   - La bocina y la baliza siguen sonando por el buzzer del pin D7.
     El DFPlayer Mini no tiene entrada de audio para el tone: sus
     pines DAC_L/DAC_R son salidas, no entradas.
   - El DFPlayer se alimenta aparte (5V externos recomendado) con
     GND comun al Arduino.

   AVISO DE BLUETOOTH
   - "Conectado" suena cuando la app manda C al conectarse.
   - "Desconectado" suena cuando tocas DESCONECTAR en la app.

   LUCES
   - Posicion: las delanteras y las traseras quedan encendidas
     siempre al 25% (analogWrite 64).
   - Faro (boton FARO de la app): las delanteras pasan al 100%.
   - Reversa: las traseras pasan al 100% mientras retrocedes y
     vuelven al 25% cuando soltas.
   - Baliza: parpadean las DOS luces de giro + beep.
   - Luz de giro: si el volante esta girado 35 grados o mas,
     parpadea SOLO la luz del lado que gira. Si te queda al reves
     (izquierda enciende la derecha), cambia GIRO_IZQ por GIRO_DER.
   - Para el 25% hacen falta pines PWM: por eso las luces van en
     D5 y D6. Ver "CABLES QUE CAMBIAN" abajo.

   CABLES QUE CAMBIAN (si venias de la version anterior)
     - ESC: de D5 a D4
     - DFPlayer RX (la resistencia de 1k): de D6 a A3
     - Luces delanteras: de D8 a D6
     - Luces traseras/reversa: cables nuevos al pin D5

   CONEXIONES
     D2  -> receptor de radio, canal DIRECCION   (entrada)
     D3  -> receptor de radio, canal ACELERADOR  (entrada)
     D4  -> ESC del motor (brushed)
     D5  -> luces traseras + reversa (PWM)
     D6  -> luces delanteras + posicion (PWM)
     D7  -> buzzer (bocina y baliza)
     D8  -> libre
     D11 -> servo de direccion
     D12 -> del modulo BT TX al Arduino (RX de BTSerial)
     D13 -> del Arduino al modulo BT RX (TX de BTSerial)
     A0  -> luz de giro 1 (lado A)
     A1  -> luz de giro 2 (lado B)
     A2  -> DFPlayer TX (no se conecta)
     A3  -> Arduino TX al DFPlayer RX (por 1k)
     A4  -> SDA del VL53L1X
     A5  -> SCL del VL53L1X
     VL53L1X: VIN a 5V, GND a GND, XSHUT sin conectar
     DFPlayer: VCC a 5V, GND a GND comun, SPK1/SPK2 al parlante

   LIBRERIAS (instalar desde el Administrador de librerias del IDE):
     Servo
     SoftwareSerial (viene con el IDE)
     Wire (viene con el IDE)
     Adafruit_VL53L1X
     DFRobotDFPlayerMini
   ─────────────────────────────────────────────────────────────
*/

#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <DFRobotDFPlayerMini.h>

// receptor de radio
#define PIN_DIR 2
#define PIN_GAS 3

// salidas
#define ESC_PIN 4
#define SERVO_PIN 11
#define BUZZ 7
#define LED_FRENTE 6         // luces delanteras / posicion (PWM)
#define LED_ATRAS 5          // luces traseras / reversa (PWM)
#define GIRO_IZQ A0
#define GIRO_DER A1

// bluetooth
#define TXD 12
#define RXD 13
SoftwareSerial BTSerial(TXD, RXD);

// dfplayer (mp3)
#define MP3_RX A2             // no se usa, pero la libreria pide los dos pines
#define MP3_TX A3             // va al RX del DFPlayer por una resistencia de 1k
SoftwareSerial mp3Serial(MP3_RX, MP3_TX);
DFRobotDFPlayerMini mp3;

Servo esc;
Servo direccion;
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();

// ajustes de la radio
#define NEUTRO 1500          // pulso neutro del ESC (si no arma, probar 1515)
#define DIR_MIN_PULSO 1148   // pulso del receptor para un extremo
#define DIR_MAX_PULSO 2100   // pulso del receptor para el otro extremo
#define ZONA_MUERTA 40       // microsegundos: menos que esto se considera neutro

// ajustes del sensor de distancia
#define DIST_FRENO 30        // cm: mas cerca que esto = frena y bloquea el avance
#define DIST_LIBRE 40        // cm: hay que alejarse hasta aca para volver a avanzar
#define GIRO_MIN 35          // grados: giro "considerable" (anti choque y luz de giro)

// estado
bool maxPotencia = false;    // false = EcoSport desactivado = mitad de potencia
bool antiChoque = false;     // false = arranca apagada (la app manda el estado al conectar)
bool balizaActiva = false;
bool estadoBaliza = false;
bool faroActivo = false;     // false = luces delanteras al 25% (posicion)
bool bloqueado = false;      // true = obstaculo cerca, no se puede avanzar
int distancia = 0;           // cm (ultima lectura del sensor)
unsigned long ultimaLectura = 0;
unsigned long previoBaliza = 0;
const unsigned long INTERVALO_BALIZA = 330;

char dato;

void setup() {
  pinMode(PIN_DIR, INPUT);
  pinMode(PIN_GAS, INPUT);

  pinMode(BUZZ, OUTPUT);
  pinMode(LED_FRENTE, OUTPUT);
  pinMode(LED_ATRAS, OUTPUT);
  pinMode(GIRO_IZQ, OUTPUT);
  pinMode(GIRO_DER, OUTPUT);

  analogWrite(LED_FRENTE, 64);    // luces de posicion al 25%
  analogWrite(LED_ATRAS, 64);
  digitalWrite(GIRO_IZQ, LOW);
  digitalWrite(GIRO_DER, LOW);

  direccion.attach(SERVO_PIN);
  direccion.write(90);            // servo al centro

  // antes del delay, asi no se pierden los estados que manda la app al conectar
  BTSerial.begin(9600);

  // dfplayer: arranca sin esperar respuesta, asi no frena el setup
  mp3Serial.begin(9600);
  mp3.begin(mp3Serial, false, false);
  BTSerial.listen();              // el bluetooth queda escuchando

  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(NEUTRO);  // neutro
  delay(3000);                    // tiempo para que el ESC arme y el DFPlayer inicie

  // sonido de arranque de motor (01/001.mp3)
  mp3.volume(20);                 // volumen de 0 a 30
  mp3.playFolder(1, 1);

  // sensor de distancia
  Wire.begin();
  vl53.begin(0x29, &Wire);
  vl53.startRanging();
  vl53.setTimingBudget(33);       // valores validos: 15, 20, 33, 50, 100, 200, 500
}

void loop() {
  // 1) distancia (solo lee cuando el sensor tiene un dato nuevo)
  leerDistancia();

  // 2) receptor de radio (timeout 25 ms)
  int pulsoDir = pulseIn(PIN_DIR, HIGH, 25000);
  int pulsoGas = pulseIn(PIN_GAS, HIGH, 25000);

  // 3) servo de direccion
  int angulo = 90;
  if (pulsoDir > 0) {
    angulo = map(pulsoDir, DIR_MIN_PULSO, DIR_MAX_PULSO, 180, 0);
    angulo = constrain(angulo, 0, 180);
  }
  direccion.write(angulo);

  // 4) motor
  int gas = NEUTRO;
  if (pulsoGas > 0) gas = pulsoGas;

  bool quiereAvanzar = gas > NEUTRO + ZONA_MUERTA;
  bool quiereRetroceder = gas < NEUTRO - ZONA_MUERTA;
  bool giroGrande = abs(angulo - 90) >= GIRO_MIN;
  bool sensorOk = (millis() - ultimaLectura) < 500;  // dato fresco (menos de 0.5 s)

  // 4a) luces: posicion al 25%; 100% con faro o retroceso
  analogWrite(LED_FRENTE, faroActivo ? 255 : 64);
  analogWrite(LED_ATRAS, quiereRetroceder ? 255 : 64);

  // 4b) anti choque: si esta activo, al detectar un objeto a menos de
  //     30 cm frena y bloquea el avance
  if (!antiChoque) {
    bloqueado = false;                       // funcion apagada: el sensor no hace nada
  } else {
    if (sensorOk && distancia > 0 && distancia < DIST_FRENO && quiereAvanzar) {
      bloqueado = true;
    }

    // 4c) el bloqueo se libera solo si el auto se alejo lo suficiente
    if (bloqueado && sensorOk && distancia > DIST_LIBRE) {
      bloqueado = false;
    }

    // 4d) bloqueado: no avanza, salvo que este girando bastante (la reversa siempre va)
    if (bloqueado && !giroGrande && !quiereRetroceder) {
      gas = NEUTRO;
    }
  }

  // 4e) EcoSport y limites
  if (!maxPotencia) {
    gas = NEUTRO + (gas - NEUTRO) / 2;   // EcoSport desactivado: mitad
  }
  gas = constrain(gas, 1000, 2000);
  esc.writeMicroseconds(gas);

  // 5) baliza y luces de giro (parpadeo con millis, sin delay)
  unsigned long ahora = millis();
  if (ahora - previoBaliza >= INTERVALO_BALIZA) {
    previoBaliza = ahora;
    estadoBaliza = !estadoBaliza;
    if (balizaActiva) tone(BUZZ, estadoBaliza ? 988 : 784, 30);
  }

  if (balizaActiva) {
    // baliza: parpadean las dos luces
    digitalWrite(GIRO_IZQ, estadoBaliza ? HIGH : LOW);
    digitalWrite(GIRO_DER, estadoBaliza ? HIGH : LOW);
  } else {
    // luz de giro: parpadea solo el lado que gira (35 grados o mas)
    bool giroIzq = angulo <= 90 - GIRO_MIN;
    bool giroDer = angulo >= 90 + GIRO_MIN;
    digitalWrite(GIRO_IZQ, (estadoBaliza && giroIzq) ? HIGH : LOW);
    digitalWrite(GIRO_DER, (estadoBaliza && giroDer) ? HIGH : LOW);
  }

  // 6) bluetooth
  if (BTSerial.available()) {
    dato = BTSerial.read();
    comandos(dato);
  }
}

void leerDistancia() {
  if (vl53.dataReady()) {
    int16_t mm = vl53.distance();
    if (mm >= 0) {
      distancia = mm / 10;      // mm a cm
      ultimaLectura = millis();
    }
    vl53.clearInterrupt();
  }
}

void comandos(char com) {
  switch (com) {
    case 'V': tone(BUZZ, 400); break;            // bocina mantenida
    case 'v': noTone(BUZZ); break;               // soltar bocina

    case 'W': faroActivo = true; break;           // faro on (100%)
    case 'w': faroActivo = false; break;          // faro off (queda en 25%)

    case 'X': balizaActiva = true; break;        // baliza on
    case 'x': balizaActiva = false;              // baliza off
              noTone(BUZZ);
              digitalWrite(GIRO_IZQ, LOW);
              digitalWrite(GIRO_DER, LOW);
              break;

    case 'P': maxPotencia = true; break;         // EcoSport: maxima potencia
    case 'p': maxPotencia = false; break;        // EcoSport: mitad

    case 'A': antiChoque = true; break;          // anti choque activado
    case 'a': antiChoque = false; break;         // anti choque desactivado

    case 'C': mp3.playFolder(2, 1); break;       // 02/001.mp3: bluetooth conectado
    case 'D': mp3.playFolder(2, 2); break;       // 02/002.mp3: bluetooth desconectado
  }
}
