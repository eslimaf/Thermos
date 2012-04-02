#include <Arial14.h>
//#include <ks0108_Panel.h>
#include <ks0108.h>
//#include <ks0108_Mega.h>
#include <ArduinoIcon.h>
//#include <ks0108_Sanguino.h>
#include <SystemFont5x7.h>
//#include <ks0108_Arduino.h>

  unsigned long startMillis;
  unsigned int iter = 0;

void setup(){

  GLCD.Init(NON_INVERTED);   // initialise the library
  GLCD.ClearScreen();  
  GLCD.DrawBitmap(ArduinoIcon, 32,0, BLACK); //draw the bitmap at the given x,y position
  delay(3000);
  GLCD.ClearScreen();
  GLCD.SelectFont(System5x7);       // select fixed width system font 

}

void loop(){ // run over and over again

  startMillis = millis();
  while( millis() - startMillis < 1000){ // loop for one second
    GLCD.DrawRect(0, 0, 64, 61, BLACK); // rectangle in left side of screen
    GLCD.DrawRoundRect(68, 0, 58, 61, 5, BLACK);  // rounded rectangle around text area   
    for(int i=0; i < 62; i += 4)
      GLCD.DrawLine(1,1,63,i, BLACK);  // draw lines from upper left down right side of rectangle  
    GLCD.DrawCircle(32,31,30,BLACK);   // draw circle centered in the left side of screen  
    GLCD.FillRect(92,40,16,16, WHITE); // clear previous spinner position  
    GLCD.CursorTo(5,5);               // locate curser for printing text
    GLCD.PrintNumber(++iter);         // print current iteration at the current cursor position 
  } 
  // display number of iterations in one second
  GLCD.ClearScreen();               // clear the screen  
  GLCD.CursorTo(13,2);              // positon cursor  
  GLCD.Puts("FPS= ");               // print a text string  
  GLCD.PrintNumber(iter);           // print a number 
 }

