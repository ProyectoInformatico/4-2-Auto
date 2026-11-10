
#include <Wire.h>
#include <VL53L0X.h>
#include <Servo.h>

VL53L0X sensor;

Servo direccion;
Servo esc;

int dir;
int gas;
int angulo;
int distancia;

void setup()
{
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  direccion.attach(9);
  esc.attach(10);

  Wire.begin();
  sensor.init();

  esc.writeMicroseconds(1515);

  delay(3000);
}

void loop()
{
  dir = pulseIn(2, HIGH);
  gas = pulseIn(3, HIGH);

  distancia = sensor.readRangeSingleMillimeters();

  angulo = map(dir, 1148, 2100, 180, 0);
  angulo = constrain(angulo, 0, 180);

  direccion.write(angulo);

  if (distancia <= 200 || distancia == 65535)
  {
    esc.writeMicroseconds(1515);
    Serial.println("elemento detectado ¡frenar!");
  }
  else
  {
    esc.writeMicroseconds(gas);
    Serial.println("libre");
  }

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" mm");

  delay(20);
}
