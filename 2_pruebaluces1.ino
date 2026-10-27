#include "BluetoothSerial.h"

BluetoothSerial serialbt;

int deli = 2;
int deld = 3;
int guii = 4;
int guid = 5;
int trasi = 6;
int trasd = 7;
int guiti = 8;
int guitd = 9;
int revi = 10;
int revd = 11;

void setup() {
  Serial.begin(115200);
  serialbt.begin("Auto_RC");

  for (int pin = 2; pin <= 11; pin++) {
    pinMode(pin, OUTPUT);
  }

  Serial.println("Bluetooth listo");
}

void loop() {
  if (serialbt.available()) {
    char dato = serialbt.read();

    if (dato == 'A') {
      digitalWrite(deli, HIGH);
      digitalWrite(deld, HIGH);
      digitalWrite(trasi, HIGH);
      digitalWrite(trasd, HIGH);
    }

    if (dato == 'B') {
      digitalWrite(deli, LOW);
      digitalWrite(deld, LOW);
      digitalWrite(trasi, LOW);
      digitalWrite(trasd, LOW);
    }

    if (dato == 'I') {
      digitalWrite(guii, HIGH);
      digitalWrite(guiti, HIGH);
    }

    if (dato == 'D') {
      digitalWrite(guid, HIGH);
      digitalWrite(guitd, HIGH);
    }

    if (dato == 'X') {
      digitalWrite(guii, LOW);
      digitalWrite(guiti, LOW);
      digitalWrite(guid, LOW);
      digitalWrite(guitd, LOW);
    }

    if (dato == 'R') {
      digitalWrite(revi, HIGH);
      digitalWrite(revd, HIGH);
    }

    if (dato == 'S') {
      digitalWrite(revi, LOW);
      digitalWrite(revd, LOW);
    }

    if (dato == 'E') {
      digitalWrite(guii, HIGH);
      digitalWrite(guid, HIGH);
      digitalWrite(guiti, HIGH);
      digitalWrite(guitd, HIGH);
    }
  }
}

