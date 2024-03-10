/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "TMPL6-4NC9ymn"
#define BLYNK_TEMPLATE_NAME "miniProject"
// #define BLYNK_AUTH_TOKEN "aq6AsuRn9OurP9_J9ENxzE68uaLTLf05"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_ESP32_DEV_MODULE
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI
#include <ESP32Servo.h>
#include "BlynkEdgent.h"
#include <TimeLib.h>
BlynkTimer timer;

Servo myservo;
int work = 0;
int Value_Fix = 3;  // จำนวนครั้งให้อาหาร
int cc = 0;
long timer_, timerChecking;
long timer_start_set[2] = { 0xFFFF, 0xFFFF };
long timer_stop_set[2] = { 0xFFFF, 0xFFFF };
unsigned char weekday_set[2];
long rtc_sec;
unsigned char day_of_week;
bool update_blynk_status[2];

//============================= ฟังก์ชั่นให้อาหาร ============================//
BLYNK_WRITE(V0) {
  int val = param.asInt();
  if (val == 1) {
    work = 1;
    Serial.println("ON");
  }
}

//============================ timer to blynk ===========================//
BLYNK_WRITE(V1) {
  unsigned char week_day;
  TimeInputParam t(param);

  if (t.hasStartTime() && t.hasStopTime()) {
    timer_start_set[0] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_set[0] = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();

    Serial.println(String("Start Time: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());

    Serial.println(String("Stop Time: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());

    for (int i = 1; i <= 7; i++) {
      if (t.isWeekdaySelected(i)) {
        week_day |= (0x01 << (i - 1));
        Serial.println(String("Day ") + i + " is selected");
      } else {
        week_day &= (~(0x01 << (i - 1)));
      }
    }
    weekday_set[0] = week_day;
  } else {
    timer_start_set[0] = 0xFFFF;
    timer_stop_set[0] = 0xFFFF;
  }
}
BLYNK_WRITE(InternalPinRTC) {
  const unsigned long DEFAULT_TIME = 1357041600;  // Jan 1 2013
  unsigned long blynkTime = param.asLong();

  if (blynkTime >= DEFAULT_TIME) {
    setTime(blynkTime);

    day_of_week = weekday();

    if (day_of_week == 1)
      day_of_week = 7;
    else
      day_of_week -= 1;

    rtc_sec = (hour() * 60 * 60) + (minute() * 60) + second();
    Serial.println(blynkTime);
    Serial.println(String("RTC Server: ") + hour() + ":" + minute() + ":" + second());
    Serial.println(String("Day of Week: ") + weekday());
  }
}
BLYNK_CONNECTED() {
  Blynk.sendInternal("rtc", "sync");
}
void checkTime() {
  Blynk.sendInternal("rtc", "sync");
}
void Logic_state() {
  bool time_set_overflow;
  for (int i = 0; i < 2; i++) {
    time_set_overflow = 0;
    if (timer_start_set[i] != 0xFFFF && timer_stop_set[i] != 0xFFFF) {
      if (timer_stop_set[i] < timer_start_set[i]) {
        time_set_overflow = 1;
      }
      if ((((time_set_overflow == 0 && (rtc_sec >= timer_start_set[i]) && (rtc_sec < timer_stop_set[i])) || (time_set_overflow && ((rtc_sec >= timer_start_set[i]) || (rtc_sec < timer_stop_set[i])))) && (weekday_set[i] == 0x00 || (weekday_set[i] & (0x01 << (day_of_week - 1)))))) {
        work = 1;
      } else {
        //        work = 0;
      }
    }
  }
}

// #########################################################################################################
void setup() {
  Serial.begin(115200);
  myservo.attach(33);
  myservo.write(10);
  BlynkEdgent.begin();
  timer.setInterval(1000L, checkTime);
}

// #########################################################################################################
void loop() {
  BlynkEdgent.run();
  timer.run();
  Logic_state();
  Serial.println(work);
  if (work == 0) {
    cc = 0;
    myservo.write(10);
  } else if (work == 1) {
    while (cc < Value_Fix) {
      Serial.println("Open > ");
      myservo.write(80);
      delay(1000);
      myservo.write(10);
      delay(1000);
      cc++;
    }
    Serial.println("END");
    delay(5000);
    work = 0;
  }
}