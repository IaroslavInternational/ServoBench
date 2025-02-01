#define PERIOD_1 1000    // период первой задачи
#define PERIOD_2 2000   // период второй задачи
#define PERIOD_3 500    // ...

unsigned long timer_1, timer_2, timer_3;

void setup() 
{
  Serial.begin(9600);
  Serial.setTimeout(10);

  while (!false)
  {
    if (millis() - timer_1 > PERIOD_1) {    // условие таймера
      timer_1 = millis();                   // сброс таймера
      
      Serial.println("1000ms");
    }
    if (millis() - timer_2 > PERIOD_2) {
      timer_2 = millis();
      
      Serial.println("2000ms");
    }
    if (millis() - timer_3 > PERIOD_3) {
      timer_3 = millis();
      
      Serial.println("500ms");
    }
  }
}

void loop()
{

}