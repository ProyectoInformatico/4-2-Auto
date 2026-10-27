
#define BUZZ 9

void arranque()
{
  tone(BUZZ, 180, 150);
  delay(200);
  tone(BUZZ, 250, 200);
  delay(250);
  tone(BUZZ, 320, 250);
  delay(300);
  noTone(BUZZ);
}

void aceleracion()
{
  tone(BUZZ, 300, 200);
  delay(200);
  tone(BUZZ, 450, 200);
  delay(200);
  tone(BUZZ, 650, 300);
  delay(300);
  noTone(BUZZ);
}

void bocina()
{
  tone(BUZZ, 500, 200);
  delay(250);
  noTone(BUZZ);
}

void setup()
{
  pinMode(BUZZ, OUTPUT);
  Serial.begin(9600);

  Serial.println("1 Arranque");
  Serial.println("2 Aceleracion");
  Serial.println("3 Bocina");
}

void loop()
{
  if (Serial.available() > 0)
  {
    char opcion = Serial.read();

    if (opcion == '1')
    {
      arranque();
    }
    else if (opcion == '2')
    {
      aceleracion();
    }
    else if (opcion == '3')
    {
      bocina();
    }
  }
}
