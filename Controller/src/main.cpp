#include <Arduino.h>
#include <PS2X_lib_temi.h>


PS2X ps2x;
byte ps2x_v=0;
int ps2x_e=1;
byte ps2x_t=0;
#define pressures false
#define rumble false

const byte Motor1A=17;
const byte Motor1B=16;
const byte Motor2A=4;
const byte Motor2B=2;
const byte Motor3A=15;
const byte Motor3B=13;
const byte Motor4A=12;
const byte Motor4B=14;

const byte MR1A=17;
const byte MR1B=16;
const byte MR2A=4;
const byte MR2B=2;
const byte MR3A=15;
const byte MR3B=13;
const byte MR4A=12;
const byte MR4B=14;

void run(int m1, int m2, int m3, int m4)
{
  int max = 230;
  if (m1 > 0)
  {
    analogWrite(Motor1A, min(abs(m1), max));
    analogWrite(Motor1B, 0);
  }
  else
  {
    analogWrite(Motor1A, 0);
    analogWrite(Motor1B, min(abs(m1), max));
  }
  
  if (m2 < 0)
  {
    analogWrite(Motor2A, min(abs(m2), max));
    analogWrite(Motor2B, 0);
  }
  else
  {
    analogWrite(Motor2A, 0);
    analogWrite(Motor2B, min(abs(m2), max));
  }
  if (m3 < 0)
  {
    analogWrite(Motor3A, min(abs(m3), max));
    analogWrite(Motor3B, 0);
  }
  else
  {
    analogWrite(Motor3A, 0);
    analogWrite(Motor3B, min(abs(m3), max));
  }
  if (m4 > 0)
  {
    analogWrite(Motor4A, min(abs(m4), max));
    analogWrite(Motor4B, 0);
  }
  else
  {
    analogWrite(Motor4A, 0);
    analogWrite(Motor4B, min(abs(m4), max));
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(Motor1A, OUTPUT);
  pinMode(Motor1B, OUTPUT);
  pinMode(Motor2A, OUTPUT);
  pinMode(Motor2B, OUTPUT);
  pinMode(Motor3A, OUTPUT);
  pinMode(Motor3B, OUTPUT);
  pinMode(Motor4A, OUTPUT);
  ps2x.config_gamepad(18, 23, 5, 19, 0, 0);
  
}
int Lx,
    Ly,
    Rx;

void loop() {
  ps2x.read_gamepad(0,ps2x_v);
  delay(16);
  
  
  Lx = map(ps2x.Analog(PSS_LX), 0, 255, -255, 255);
  Ly = map(ps2x.Analog(PSS_LY), 0, 255, 255, -255);
  Rx = map(ps2x.Analog(PSS_RX), 0, 255, -255, 255);
  run(Ly+Lx+Rx, Ly-Lx-Rx, Ly+Lx-Rx, Ly-Lx+Rx);
}