/**********************************************************************************
* The MIT License
* 
* Copyright (c) 2011 - 2012 Evandro Lima
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************************/
#include <SoftwareSerial.h>
#include <string.h>
#include <glcd.h>
#include <fonts/allFonts.h>

#define MEASURE_TEMP 5000
#define THRESHOLD 32
#define SMS_NUMBER 81664088
#define REPORT_TIME 300000
#define BUFFERSIZE 90

#define DEBUG 0

#define ACTION_REGISTER 0x01
#define ACTION_CHECK_SIGNAL 0x02
#define ACTION_SEND_SMS 0x03

char at_buffer[BUFFERSIZE];
int buffidx;

char incoming_char = 0;
//char buffer[60];

byte termoPin = 5;
SoftwareSerial modem(2,3);
long previousMillis = 0;
long lastSMSMillis = 0;
float tempV = 0.0;

byte SMS_Ready = 0;
byte GPRS_Registered = 0;

boolean isModemReady = false;
boolean needScreenRefresh = true;

gText infoArea = gText(2,14,GLCD.Width-2,GLCD.Height-2);

/**
  Read Modem Output
*/
void ReadModemOutput(void) {
  char c;
  buffidx= 0; // start at begninning
  while (1) {
    if(modem.available() > 0) {
      c = modem.read();
      if (c == -1) {
        at_buffer[buffidx] = '\0';
        return;
      }
 
      if (c == '\n') {
        continue;
      }
 
      if ((buffidx == BUFFERSIZE - 1) || (c == '\r')){
        at_buffer[buffidx] = '\0';
        return;
       }
 
      at_buffer[buffidx++]= c;
    }
  }
}

/**
  ParseAtCommand
*/
void ParseAtCommand() {
 
  if( strstr(at_buffer, "+SIND: 8") != 0 ) {
    GPRS_Registered = 0;
    infoArea.println("GPRS Network Not Available");
  }

  if( strstr(at_buffer, "+SIND: 11") != 0 ) {
    GPRS_Registered=1;
    infoArea.println("GPRS Registered");
  }
 
  if( strstr(at_buffer, "+SIND: 4") != 0 ) {
    SMS_Ready = 1;
    isModemReady = true;
    infoArea.println("SMS is Ready");
  }
}

void SendATCommand(byte at_cmd){
  switch(at_cmd){
    //Register
    case ACTION_REGISTER: break;
    //Check signal strengh
    case ACTION_CHECK_SIGNAL: break;
    //Send SMS
    case ACTION_SEND_SMS: break;

  }
}

//Send SMS
void SendSMS(char *msg) {
  modem.println("AT+CMGF=1");
  delay(500);
  modem.print("AT+CMGS=");
  modem.print("\"");
  modem.print(SMS_NUMBER);
  modem.println("\"");
  delay(500);                // Wait for ">" prompt
  modem.print(msg);
  modem.print(" ");
  modem.print(tempV);
  modem.print("C");
  modem.write(0x1A);   // CTRL-Z
  while(true){
    incoming_char = modem.read();
    if(incoming_char == 'O') {
      infoArea.print(incoming_char);
      incoming_char = modem.read();
      if(incoming_char == 'K') {
        infoArea.print(incoming_char);
        return;
      }
    }
  }
  modem.println("AT+CMGS=1,4");
}

/**
  Basic screen layout
*/
void RefreshScreen(){
  //Print statusbar
	GLCD.GotoXY(0,0);
        //Print NETWORK AND SIGNAL
	GLCD.print("TIM");
  GLCD.DrawRect(20,5,1,1);
  GLCD.DrawRect(23,4,1,2);
  GLCD.DrawRect(26,3,1,3);
  GLCD.DrawRect(29,2,1,4);
  GLCD.DrawRect(32,1,1,5);
  //Print Battery status
  GLCD.DrawRect(109,3,1,2);
  GLCD.DrawRect(112,1,1,6);
  GLCD.DrawRect(115,1,1,6);
  GLCD.DrawRect(118,1,1,6);
  GLCD.DrawRect(121,1,1,6);
  //Draw separator
	GLCD.DrawHLine(0,9,GLCD.Width-1);
  //Draw informations
  //GlcdWriteInfo(info);
	needScreenRefresh = false;
}

void setup()
{
  Serial.begin(9600);
  modem.begin(9600);
  GLCD.Init();
  GLCD.SelectFont(System5x7);
  infoArea.SelectFont(System5x7);
  infoArea.println("Starting SM5100B");
  RefreshScreen();
}

void loop()
{
  long currentMillis = millis();
  if(currentMillis - previousMillis > MEASURE_TEMP) {
    previousMillis = currentMillis;
    tempV = (5.0 * analogRead(termoPin) * 100.0) / 1024.0;
    if (DEBUG) Serial.println(tempV);
    if(isModemReady & (tempV > THRESHOLD)) {
      SendSMS("(Alert)");
      infoArea.println("Alert Sending SMS");
    }
  }

  if(isModemReady & (currentMillis - lastSMSMillis > REPORT_TIME)){
    lastSMSMillis = currentMillis;
    SendSMS("(Report)");
    infoArea.println("Reporting...");
  }
  if(modem.available() > 0)
  {
    if(!isModemReady){
      /*incoming_char=modem.read();
      if(incoming_char == '4') {
        isModemReady = true;
      }
      infoArea.print(incoming_char);*/
      ReadModemOutput();
      ParseAtCommand();

    }else{
      incoming_char=modem.read();
      infoArea.print(incoming_char);
    }
  }

  if(Serial.available() > 0)
  {
    incoming_char=Serial.read();
    if(isModemReady)
    modem.print(incoming_char);
  }
  if(needScreenRefresh){
  //RefreshScreen("");
  }
}

