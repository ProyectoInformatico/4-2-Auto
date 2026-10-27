

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

char comando;

bool luces = false;
bool reversa = false;
bool balizas = false;
bool izq = false;
bool der = false;

void setup() {
  Serial.begin(9600);

  for (int pin = 2; pin <= 11; pin++) {
    pinMode(pin, OUTPUT);
  }
}

void loop() {
  if (Serial.available() > 0) {
    comando = Serial.read();

    if (comando == 'L') {
      luces = !luces;
      digitalWrite(deli, luces);
      digitalWrite(deld, luces);
      digitalWrite(trasi, luces);
      digitalWrite(trasd, luces);
    }

    if (comando == 'R') {
      reversa = !reversa;
      digitalWrite(revi, reversa);
      digitalWrite(revd, reversa);
    }

    if (comando == 'I') {
      izq = true;
      der = false;
      balizas = false;
    }

    if (comando == 'D') {
      der = true;
      izq = false;
      balizas = false;
    }

    if (comando == 'A') {
      balizas = !balizas;
      izq = false;
      der = false;
    }

    if (comando == 'X') {
      balizas = false;
      izq = false;
      der = false;
    }
  }

  if (balizas) {
    digitalWrite(guii, HIGH);
    digitalWrite(guid, HIGH);
    digitalWrite(guiti, HIGH);
    digitalWrite(guitd, HIGH);
    delay(500);

    digitalWrite(guii, LOW);
    digitalWrite(guid, LOW);
    digitalWrite(guiti, LOW);
    digitalWrite(guitd, LOW);
    delay(500);
  }
  else if (izq) {
    digitalWrite(guii, HIGH);
    digitalWrite(guiti, HIGH);
    delay(500);
    digitalWrite(guii, LOW);
    digitalWrite(guiti, LOW);
    delay(500);
  }
  else if (der) {
    digitalWrite(guid, HIGH);
    digitalWrite(guitd, HIGH);
    delay(500);
    digitalWrite(guid, LOW);
    digitalWrite(guitd, LOW);
    delay(500);
  }
  else {
    digitalWrite(guii, LOW);
    digitalWrite(guid, LOW);
    digitalWrite(guiti, LOW);
    digitalWrite(guitd, LOW);
  }
}

