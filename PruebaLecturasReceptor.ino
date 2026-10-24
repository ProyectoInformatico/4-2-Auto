// Prueba De Lectura de Receptor con Radio
void setup()
{
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop()
{
  unsigned long direccion = pulseIn(2, HIGH);
  unsigned long acelerador = pulseIn(3, HIGH);

  Serial.print("DIR: ");
  Serial.print(direccion);

  Serial.print("    GAS: ");
  Serial.println(acelerador);

  delay(100);
}
