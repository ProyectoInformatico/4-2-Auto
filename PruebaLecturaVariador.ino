// Prueba para detectar correctamente la aceleración (adelante), la reversa y el punto muerto (neutro)
void setup()
{
  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop()
{
  int gas = pulseIn(3, HIGH);

  Serial.print("Valor: ");
  Serial.print(gas);
  Serial.print(" ---> ");

  if(gas < 1450)
{
    Serial.println("ADELANTE");
}
else if(gas > 1580)
{
    Serial.println("ATRAS");
}
else
{
    Serial.println("NEUTRO");
}

  delay(100);
}
