#include <Arduino.h>
#include <U8g2lib.h>
#include <Timer.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

void startExposure();
void stopExposure();
void countDown();
void displayTimer(int val);

#define ENCODER_MAX 3600
#define ENCODER_MIN 0
#define ENCODER_START 90

#define TIMER_MIN 0
#define TIMER_MAX 3600

#define ENCODER_PIN_A PIN6
#define ENCODER_PIN_B PIN7
#define BUTTON_PIN PIN2
#define SWITCH_PIN PIN3

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

int timerValue = 0;
Timer t;
bool underExposure = false;
int countEvent, exposureEvent;
char buffer[6];
int counter;
static int encoderPos = ENCODER_START;
ClickEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B, BUTTON_PIN);


void timerIsr() {
  encoder.service();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT);
  pinMode(ENCODER_PIN_B, INPUT);

  pinMode(SWITCH_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), startExposure, FALLING);
  
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setFont(u8g2_font_profont22_tn);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  encoder.setAccelerationEnabled(true);
}

void loop() {
  t.update();

  if(!underExposure) {
    encoderPos += (encoder.getValue() * 10);

    timerValue = map(encoderPos, ENCODER_MIN, ENCODER_MAX, TIMER_MIN, TIMER_MAX);
    timerValue = constrain(timerValue, TIMER_MIN, TIMER_MAX);
    
    displayTimer(timerValue);
  }
}

void displayTimer(int val) {
  int minutes = val / 60;
  int seconds = val - (minutes * 60);
  int barSize = map(val, TIMER_MIN, TIMER_MAX, 1, 126);
  
  sprintf(buffer, "%02d:%02d", minutes, seconds);
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0,0,127,30);
    u8g2.drawBox(2,2,barSize,28);
    u8g2.setCursor(32,63);
    u8g2.print(buffer);
  } while( u8g2.nextPage());
}

void startExposure() {
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
  digitalWrite(SWITCH_PIN, HIGH);
  underExposure = true;
  exposureEvent = t.after(timerValue * 1000L, stopExposure);
  countEvent = t.every(1000L, countDown);
}

void stopExposure() {
  digitalWrite(SWITCH_PIN, LOW);
  underExposure = false;
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), startExposure, FALLING);
  t.stop(countEvent);
  t.stop(exposureEvent);
}

void countDown() {
  timerValue--;
  displayTimer(timerValue);
}
