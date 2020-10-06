//We always have to include the library
#include "LedControl.h"
#include "SmartDelay.h"

/*
  Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
  pin 12 is connected to the DataIn
  pin 11 is connected to the CLK
  pin 10 is connected to LOAD
  We have only a single MAX72XX.
*/
LedControl lc = LedControl(12, 11, 10, 1);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 250;
unsigned int divisor = 1; // сколько отсчётов на 1 оборот
unsigned int limit = 16000; // максимально оборотов в минуту

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);
  digitalWrite(13, LOW);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  pinMode(2, INPUT); // tacho impulses here
  attachInterrupt(digitalPinToInterrupt(2), getInt, RISING); //задать обработчик прерывания по фронту
}

unsigned long xx = 0;

// target is 0 - 30000 rpm = 500 rps = 2 ms
unsigned long oldk = 0; // old micros

volatile unsigned int frq = 0; // must be VOLATILE

void getInt() {
  unsigned long k = micros();
  unsigned int f;
  long dk = k - oldk;
  // Здесь неправильно считается частота, надо более точно считать, потом переделаю.
  if (oldk && oldk < k) {
    if (dk > 0 && dk < ((unsigned long)1000 * 1000 * divisor)) { // 1s - время, когда нет ни отного импульса и это как бы частота = 0
      f = (unsigned long)(1.0f * 1000 * 1000 * 60 / (float)divisor / (float)dk); // divisor импульсов на оборот (датчик такой)
      //Serial.println("calc");
    } else {
      f = 0;
      //Serial.println("zero");
    }
    frq = f;
  }
  digitalWrite(13, xx++ % 2); // для краслты мигаем диодом
  oldk = k;
}

unsigned long iv = 1000; // частота записи в лог (мс)
unsigned long ot = 0; // когда последний раз писали в лог (мс)

int bl = 0;
int bh = 0;

SmartDelay blink(1000UL*delaytime); // 250ms период горения мигалки. blink.Now() будет True наз в 250ms.
int diods=16; // количество диодов
int blocks=(diods+7)/8; // количество блоков по 8.

void loop() {
  if (millis() - ot > iv) { // писать в лог частоту раз в iv миллисекунд
    ot = millis();
    Serial.println(frq);
  }
  
  // Приводим частоту к количеству диодов
  int d = map(frq, 0, limit, 0, diods); // привести частоту к количеству диодов для мигания
  // светим диоды
  for (int i = 0; i < blocks; i++) {
    int shift=0;  // Сдвиг горящих диодов в блоке
    if (d/8 < i) shift=0; // пусть все горят
    elsif (d/8 == i) shift=8-d%8; // сдвигаем часть
    else shift=8; // все погасить
    lc.setRow(0, 0, (0xFF << shift) & 0xFF); // зажигаем 8-shift диобов в строчке.
  }
  
  // Здесь мигаем, когда надо
  
  // Если светятся 2-4 диода, то мигаем раз в 1/4 секунды
  if (2 < d && d <= 4 && blink.Now()) {
    if (bl) {
      lc.setLed(0, 2, 1, HIGH);
      bl = 0;
    } else {
      lc.setLed(0, 2, 1, LOW);
      bl = 1;
    }
  } else {  // Выключаем мигание
    lc.setLed(0, 2, 1, LOW);
    bl = 0;
  }
  
  // Если светится старший диод, то им имгаем
  if (d >= 7 && blink.Now()) {
    if (!bh) {
      lc.setLed(0, 2, 2, HIGH);
      bh = 1;
    } else {
      lc.setLed(0, 2, 2, LOW);
      bh = 0;
    }
  } else { // Выключаем мигание
    lc.setLed(0, 2, 2, LOW);
    bh = 0;
  }
}
