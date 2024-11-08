#include <Arduino.h>

#define LEFT_TRACK 33
#define RIGHT_TRACK 39

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

bool is_left_tracker_on_line()
{
  return (analogRead(LEFT_TRACK) > 330 ? true:false);
}

bool is_right_tracker_on_line()
{
  return (analogRead(RIGHT_TRACK) > 330 ? true:false);
}

void stop()
{
  run(0, 0, 0, 0);
}

int Lx,
    Ly,
    Rx;

void forward(int speed, int time = 0)
{
  run(speed, speed, speed, speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void backward(int speed, int time = 0)
{
  run(-speed, -speed, -speed, -speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void right_turn(int speed, int time = 0)
{
  run(speed, -speed, -speed, speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void left_turn(int speed, int time = 0)
{
  run(-speed, speed, speed, -speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void rightward(int speed, int time = 0)
{
  run(speed, -speed, speed, -speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void leftward(int speed, int time = 0)
{
  run(-speed, speed, -speed, speed);
  if(time != 0)
  {
    delay(time);
    stop();
  }
}

void follow(int speed, int pass_time)
{
  while (!(is_left_tracker_on_line() && is_right_tracker_on_line()))
  {
    if (is_left_tracker_on_line())
    {
      left_turn(speed);
    }
    else if (is_right_tracker_on_line())
    {
      right_turn(speed);
    }
    else
    {
      forward(speed);
    }
  }
  stop();
  delay(200);
  forward(speed);
  delay(pass_time);
  stop();
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
  
}

void loop() {
  follow(210, 600);
}