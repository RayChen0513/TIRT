/*
Modify: 2022/02/18  調整時序
*/
#include "PS2X_lib_temi.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "Arduino.h"

static byte  polling_device[5]={0x01,0x42,0x00,0x01,0x00};
static byte    enter_config[5]={0x01,0x43,0x00,0x01,0x00};
static byte     exit_config[9]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static byte        set_mode[9]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static byte set_bytes_large[9]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
static byte   enable_rumble[5]={0x01,0x4D,0x00,0x00,0x01};
//static byte       type_read[9]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};
static byte       ps2RxDef[21]={0x00,0x73,0x00,0xFF,0xFF,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
/****************************************************************************************/
PS2X::PS2X() {}
PS2X::~PS2X() {}
/****************************************************************************************/
bool PS2X::NewButtonState() {
  return ((last_buttons ^ buttons) > 0);
}
/****************************************************************************************/
bool PS2X::NewButtonState(unsigned int button) {
  return (((last_buttons ^ buttons) & button) > 0);
}
/****************************************************************************************/
bool PS2X::ButtonPressed(unsigned int button) {
  return(NewButtonState(button) & Button(button));
}
/****************************************************************************************/
bool PS2X::ButtonReleased(unsigned int button) {
  return((NewButtonState(button)) & ((~last_buttons & button) > 0));
}
/****************************************************************************************/
bool PS2X::Button(uint16_t button) {
  return ((~buttons & button) > 0);
}
/****************************************************************************************/
unsigned int PS2X::ButtonDataByte() {
   return (~buttons);
}
/****************************************************************************************/
byte PS2X::Analog(byte button) {
   return PS2data[button];
}
/****************************************************************************************/
unsigned char PS2X::_gamepad_shiftinout (char byte) {
   unsigned char tmp = 0;
   for(unsigned char i=0;i<8;i++) {
      if(CHK(byte,i)) 
          CMD_SET();
      else            
          CMD_CLR();	  
      CLK_CLR();
      delayMicroseconds(CTRL_CLK);
      if(DAT_CHK()) 
          bitSet(tmp,i);
      CLK_SET();
      delayMicroseconds(CTRL_CLK);
   }
   delayMicroseconds(CTRL_BYTE_DELAY);
   CMD_SET();
   delayMicroseconds(CTRL_BYTE_DELAY);
   return tmp;
}
/****************************************************************************************/
bool PS2X::read_gamepad() {
   return read_gamepad(false, 0x00);
}
/****************************************************************************************/
bool PS2X::read_gamepad(bool motor1, byte motor2) {

   if(motor2 != 0x00)
      motor2 = map(motor2,0,255,0x40,0xFF); //noting below 40 will make it spin

   byte dword[9] = {0x01,0x42,0,motor1,motor2,0,0,0,0};
   byte dword2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
   // Try a few times to get valid data...
   CMD_SET();
   CLK_SET();
   ATT_CLR(); // low enable joystick

   delayMicroseconds(CTRL_BYTE_DELAY);
    //Send the command to send button and joystick data;
   for (int i = 0; i<9; i++) {
        SPIRX[i] = _gamepad_shiftinout(dword[i]);
   }

   if(SPIRX[1] == 0x79) {  //if controller is in full data return mode, get the rest of data
        for (int i = 0; i<12; i++) {
            SPIRX[i+9] = _gamepad_shiftinout(dword2[i]);
        }
   } else {
       for (int i = 0; i<12; i++) {
            SPIRX[i+9] = 0;
       }
   }
   ATT_SET(); // HI disable joystick

   if ((SPIRX[1] & 0xf0) == 0x70) {
        _RetryCnt = 0;
        //if(_ConntCnt < 10 ) _ConntCnt ++;
        if(_ConntCnt > 3 ) {
           for(int i=0;i<21;i++) PS2data[i] = SPIRX[i];
           last_buttons = buttons;            // store the previous buttons states
           buttons = *(uint16_t*)(SPIRX+3);   // store as one value for multiple functions
        } else {
            _ConntCnt ++;
        }
   } else {
	    _RetryCnt++;
        _ConntCnt =0;
        for (int i = 0; i<21; i++) {      // Clear Buffer
            SPIRX[i] = 0;
        }
	    if (_RetryCnt > 10) {
		    reconfig_gamepad();           // try to get back into Analog mode.   
		    PS2_reTry = 0;	  
	    }
   }
   //if(_ConntCnt > 3 ) {
   //    for(int i=0;i<21;i++) PS2data[i] = SPIRX[i];
   //    last_buttons = buttons;            // store the previous buttons states
   //    buttons = *(uint16_t*)(SPIRX+3);   // store as one value for multiple functions
   //}
   return ((SPIRX[1] & 0xf0) == 0x70);  // 1 = OK = analog mode - 0 = NOK
}
/****************************************************************************************/
byte PS2X::config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat)
{
   return config_gamepad(clk, cmd, att, dat, false, false);
}
/****************************************************************************************/
byte PS2X::config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble) 
{
  _clk_pin = clk; 
  _cmd_pin = cmd; 
  _att_pin = att; 
  _dat_pin = dat; 

  pinMode(clk, OUTPUT); //configure ports
  pinMode(att, OUTPUT);
  pinMode(cmd, OUTPUT);
  pinMode(dat, INPUT);

  digitalWrite(dat, HIGH); //enable pull-up

  CMD_SET();
  CLK_SET();
  ATT_SET(); 
  en_Rumble = rumble; 
  en_Pressures = pressures;   
  return config_gamepad();
}
/****************************************************************************************/
byte PS2X::config_gamepad() 
{
  PS2_reTry = 0;
  reconfig_gamepad();
  read_gamepad();
  delay(16);
  return 0; //no error if here
}
/****************************************************************************************/
void PS2X::sendCommandString(byte string[], byte len) {
      ATT_CLR();
      delayMicroseconds(20);
      for (int y=0; y < len; y++)
            _gamepad_shiftinout(string[y]);
      ATT_SET(); 
}
/****************************************************************************************/
byte PS2X::readType() {
      //if(controller_type == 0x03)
      //  return 1;
      //else if(controller_type == 0x01)
      //  return 2;
      //else if(controller_type == 0x0C)  
      //  return 3;  //2.4G Wireless Dual Shock PS2 Game Controller	
      //return 0;
      return 1;
}
/****************************************************************************************/
void PS2X::reconfig_gamepad(){  
      sendCommandString(polling_device, 5);
      delay(16);
      sendCommandString(polling_device, 5);
      delay(16);
      sendCommandString(enter_config, 5);
      delay(16);
      sendCommandString(set_mode, 9);
      delay(16);
      if (en_Rumble) {
        sendCommandString(enable_rumble, 5);
        delay(16);
      }
      if (en_Pressures) {
        sendCommandString(set_bytes_large, 9);
        delay(16);
      }
      //sendCommandString(type_read, 9);
      //controller_type = SPIRX[3];
      //delay(16);
      sendCommandString(exit_config, 9);
      delay(16);
      for(int i=0;i<21;i++) PS2data[i]=ps2RxDef[i];
      last_buttons = 0xFFFF;
      buttons = 0xFFFF;
      _ConntCnt =0;
}
/****************************************************************************************/
/*   主要修改                                                                           */
/****************************************************************************************/
inline void  PS2X::CLK_SET(void) {  
  digitalWrite(_clk_pin, HIGH);
}
inline void  PS2X::CLK_CLR(void) {  
  digitalWrite(_clk_pin, LOW);
}
inline void  PS2X::CMD_SET(void) {  
  digitalWrite(_cmd_pin, HIGH);
}
inline void  PS2X::CMD_CLR(void) {  
  digitalWrite(_cmd_pin, LOW);
}
inline void  PS2X::ATT_SET(void) {  
  digitalWrite(_att_pin, HIGH);
}
inline void PS2X::ATT_CLR(void) {  
  digitalWrite(_att_pin, LOW);
}
inline bool PS2X::DAT_CHK(void) {  
  return (digitalRead(_dat_pin)) ? true : false;  
}