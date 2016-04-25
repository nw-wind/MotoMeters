//We always have to include the library
#include "LedControl.h"

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
 
  if (oldk && oldk < k) {
    if (dk > 0 && dk < ((unsigned long)1000 * 1000 * 4)) { // 1s - время, когда нет ни отного импульса и это как бы частота = 0
      f = (unsigned long)(1.0f * 1000 * 1000 * 60 / 4 / (dk)); // 4 импульса на оборот (датчик такой)
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

int s = 0;
int x = 0;
long ot = 0;
long c = 0;
long iv = 1000;
long f = 0;

int bt = 0;
int bl = 0;
int bh = 0;

void loop() {
  if (millis() - ot > iv) { // писать в лог частоту раз в iv миллисекунд
    ot = millis();
    Serial.println(frq);
  }
  int d = map(frq, 0, 300, 0, 8); // привести частоту к количеству диодов для мигания
  // светим диоды
  for (int i = 0; i < 2; i++) {
    lc.setRow(0, 0, 0xFF << (8 - d));
  }
  // магия
  int t = millis();
  if (t - bt > 250) {
    bt = t;
    if (2 < d && d <= 4) {
      if (bl) {
        lc.setLed(0, 2, 1, HIGH);
        bl = 0;
      } else {
        lc.setLed(0, 2, 1, HIGH);
        bl = 1;
      }
    } else {
      lc.setLed(0, 2, 1, LOW);
      bl = 0;
    }
    if (d >= 7) {
      if (!bh) {
        lc.setLed(0, 2, 2, HIGH);
        bh = 1;
      } else {
        lc.setLed(0, 2, 2, LOW);
        bh = 0;
      }
    } else {
      lc.setLed(0, 2, 2, LOW);
      bh = 0;
    }
  }
}
