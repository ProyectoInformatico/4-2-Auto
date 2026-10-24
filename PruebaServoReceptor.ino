// Prueba de Control de Servo Usando Receptor de Radio
#include <Servo.h>

Servo direccion;

void setup()
{
  Serial.begin(9600);

  pinMode(2, INPUT);

  direccion.attach(9);
}

void loop()
{
  int valorRC = pulseIn(2, HIGH);

  int angulo = map(
    valorRC,
    1148,
    2100,
    180,
    0
  );

  angulo = constrain(angulo, 0, 180);

  direccion.write(angulo);

  Serial.print("RC: ");
  Serial.print(valorRC);

  Serial.print("  ANG: ");
  Serial.println(angulo);

  delay(20);
}
