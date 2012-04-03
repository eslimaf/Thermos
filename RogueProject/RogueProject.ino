#include <SoftwareSerial.h>
#include <string.h>
#include <glcd.h>
#include <fonts/allFonts.h>

#define MEASURE_TEMP 5000
#define THRESHOLD 32
#define SMS_NUMBER 81664088
#define REPORT_TIME 300000
#define BUFFERSIZE 90


char at_buffer[BUFFERSIZE];
int buffidx;

char incoming_char = 0;
//char buffer[60];

byte termoPin = 5;
SoftwareSerial modem(2,3);
long previousMillis = 0;
long lastSMSMillis = 0;
float tempV = 0.0;

byte GPRS_AT_Ready = 0;
byte GPRS_Registered = 0;

boolean isModemReady = false;

//Read Modem Output
void ReadModemOutput(void) {
  char c;
  buffidx= 0; // start at begninning
  while (1) {
    if(modem.available() > 0) {
      c=modem.read();
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

void ParseAtCommand() {
 
if( strstr(at_buffer, "+SIND: 8") != 0 ) {
GPRS_Registered = 0;
Serial.println("GPRS Network Not Available");
}
 
if( strstr(at_buffer, "+SIND: 11") != 0 ) {
GPRS_Registered=1;
Serial.println("GPRS Registered");
//blinkLed(redLedPin,5,100);
 
}
 
if( strstr(at_buffer, "+SIND: 4") != 0 ) {
GPRS_AT_Ready = 1;
Serial.println("GPRS AT Ready");
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
      GLCD.print(incoming_char);
      incoming_char = modem.read();
      if(incoming_char == 'K') {
        GLCD.print(incoming_char);
        return;
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);
  modem.begin(9600);
  GLCD.Init();
  GLCD.SelectFont(System5x7);
  GLCD.println("Starting SM5100B");
}

void loop()
{
  long currentMillis = millis();
  if(currentMillis - previousMillis > MEASURE_TEMP) {
    previousMillis = currentMillis;
    tempV = (5.0 * analogRead(termoPin) * 100.0) / 1024.0;
    Serial.println(tempV);
    if(isModemReady & (tempV > THRESHOLD)) {
      SendSMS("(Alert)");
      GLCD.println("Alert Sending SMS");
    }
  }
  
  if(isModemReady & (currentMillis - lastSMSMillis > REPORT_TIME)){
    lastSMSMillis = currentMillis;
    SendSMS("(Report)");
    GLCD.println("Reporting...");
  }
  if(modem.available() > 0)
  {
    if(!isModemReady){
      incoming_char=modem.read();
      if(incoming_char == '4') {
        isModemReady = true;
        delay(1000);
      }
      GLCD.print(incoming_char);
    }else{
      incoming_char=modem.read();
      GLCD.print(incoming_char);
    }
  }
  
  if(Serial.available() > 0)
  {
    incoming_char=Serial.read();
    if(isModemReady)
    modem.print(incoming_char);
  }
}

