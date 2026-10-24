// Prueba De control Total de Motor y Servo con Radio
#include <Servo.h>

Servo direccion;
Servo esc;

void setup()
{
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  direccion.attach(9);
  esc.attach(10);

  // Neutro para que el ESC arme correctamente
  esc.writeMicroseconds(1515);

  delay(3000);
}

void loop()
{
  int dir = pulseIn(2, HIGH);
  int gas = pulseIn(3, HIGH);

  int angulo = map(
    dir,
    1148,
    2100,
    180,
    0
  );

  angulo = constrain(angulo, 0, 180);

  direccion.write(angulo);

  esc.writeMicroseconds(gas);

  Serial.print("DIR: ");
  Serial.print(dir);

  Serial.print("   GAS: ");
  Serial.println(gas);

  delay(20);
}
