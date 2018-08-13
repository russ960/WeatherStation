// receiver.pde
//
// Simple example of how to use VirtualWire to receive messages
// Implements a simplex (one-way) receiver with an Rx-B1 module
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: receiver.pde,v 1.3 2009/03/30 00:07:24 mikem Exp $
// 
// Modification History:
// Russell Johnson		08/04/2018	Modified startup information and renamed in english non-english terms.  Converted to F from C.
// Russell Johnson		08/12/2018	Adding RTC and required code.

#include <VirtualWire.h>
#include <stdio.h>
#include <string.h>
#include <LiquidCrystal.h>
#include <RTClib.h>

double Temp;
RTC_DS1307 RTC;


LiquidCrystal lcd(12, 10, 5, 4, 3, 2); //LCD Pins
int LCDpin =9;  //LCD backlight Pin
int Humid; // Value
#define pir A0 //Pin for the PIR input

//Varaible to store the recoverd values
String TempC; //Temp in C
String Humidity; //humidity
String Pressure;  //pressure
String outputTime; //Time
int hr24Time; // 24 hour clock reading
int hr12Time; // 12 hour conversion
String ampm; // 12 hour am or pm setting.

// Variable to store temp in F
String TempF; // Temp in F

void setup()
{
  Serial.begin(9600); // Debugging only
  Serial.println("setup");
  if (! RTC.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  RTC.adjust(DateTime(__DATE__, __TIME__));
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This will reflect the time that your sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
    //RTC.adjust(DateTime(2018,8,12,18,48,0));
  }
  ////Setting pins for backlight
  pinMode(pir, INPUT);
  pinMode(LCDpin, OUTPUT);

  //LCD
  lcd.begin(20, 4); //Setting rows an colums

  lcd.setCursor(0, 0); //Set Cursor at begining
  lcd.print("                    "); //Drawing

  lcd.setCursor(0, 1); //Set cursor at next line
  lcd.print("      Russell's     "); //Drawim 

  lcd.setCursor(0, 2); //Set cursor at next line
  lcd.print("  Weather  station  "); //Drawim 

  lcd.setCursor(0, 3); //Set cursor at next line
  lcd.print("        V1.0        "); //Drawim 

  //Show on Screen, if this isnt called, screen would be empty
  lcd.display(); 

  //Set back color to full brightness
  digitalWrite(LCDpin,255);

  //Wait a seccond
  delay(1000);

  //For Receiving  Data
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);      // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}



void loop()
{
  //IF PIR sensor if HIGH, LCD backlight is on
  //The Sensitivity  and duration is adjusted ON the PIR
  digitalWrite(LCDpin, digitalRead(pir));

  // Get time from RTC and assign to variable for display.
  DateTime now = RTC.now();
  outputTime = now.month();
  outputTime = outputTime + "/";
  outputTime = outputTime + now.day();
  outputTime = outputTime + '/';
  outputTime = outputTime + now.year();
  outputTime = outputTime + ' ';
  // Correct for 12 hour clock over 24 hour time.
  hr24Time = now.hour();
  if (hr24Time > 12)
    {
        hr12Time = hr24Time - 12;
        ampm = "PM";
    }
  else
  {
    hr12Time = hr24Time;
    if (hr24Time == 12)
    {
      ampm = "PM";
    }
    else if (hr24Time == 0)
    {
      hr12Time = 12;
      ampm = "AM";
    }
    else
    {
      ampm = "AM";
    }
  }
  outputTime = outputTime + hr12Time;
  outputTime = outputTime + ':';
  // Correct for missing 0 if time is less than 10.
  if (now.minute() < 10)
  {
    outputTime = outputTime + '0';
    outputTime = outputTime + now.minute();
  }
  else 
  {
    outputTime = outputTime + now.minute();
  }
  outputTime = outputTime + ' ' + ampm;
  //Serial.println(outputTime);
  
  ////Receiving  DATA
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    digitalWrite(13, true);  //onboard LED ON 
    char Chars[buflen]; 
    for (i = 0; i < buflen; i++)
    {
      Chars[i]  = char(buf[i]); //from buffer to Char Arry
    }

    Serial.println(Chars); //Debug only
    Decode(Chars); //Send to deconder
    digitalWrite(13, false); //onboard LED OFF 
  }
}

void Decode(char* Raw)
{

  String Code = (String)Raw; //From Char Array to string
  if (Code.startsWith("#C")) //Looks if Code starts With #C
  {
    TempC = Code.substring(2,7); //get 2 to 7 char and put in int TempC string
    TempF = (TempC.toFloat() * 1.8) + 32;
  }

  if (Code.startsWith("#H")) //Looks if Code starts With #H
  {
    Humidity = Code.substring(2,4);//get 2 to 4 char and put in int Humidity string
  }

  if (Code.startsWith("#P")) //Looks if Code starts With #P
  {
    Pressure =  Code.substring(2,8);//get 2 to 8 char and put in int Pressure string
  }

  SetScreen();  //Call Funtion
}

//Herbouw scherm
void SetScreen()
{

  lcd.clear(); //Cleas LCD
  lcd.begin(20, 4); //set screen res

  if (TempC != "")
  {
    
    lcd.setCursor(0, 0); //Set Cursos
    lcd.print(((String)"  Temp = " + TempF + " oF")); //Writes text with received value
  }

  if (Humidity != "")
  {
    lcd.setCursor(0, 1); //Set Cursos
    lcd.print(((String)"  Humidity = " + Humidity +  "%")); //Writes text with received value
  }

  if (Pressure != "")
  {
    lcd.setCursor(0, 2); //Set Cursos
    lcd.print(((String)"  Pressure = " + Pressure));//Writes text with received value
  }
  
  if (outputTime != "")
  {
  lcd.setCursor(0,3); //Set Cursor
	lcd.print("  " + outputTime); //Write time from RTC
  }
  lcd.display();
}

