#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2


#define LOW_END 30
#define HIGH_END 45

float currentTarget = 36;

#define POT A0
#define RELAY 10

#define DELTA_T 3


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


float temperature[50];
float average = 0;
int count = 0;

//float target[5];
float targavg = 0;

bool isOn = false;

float map_float(float x, float a, float b, float A, float B)
{
  float m = (B-A)/(b-a);
  float y = (x - a)*m + A;

  return y;
}

void setup()
{
  lcd.init();  
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Initializing");
  lcd.setCursor(1,1);
  lcd.print("Please wait...");

  pinMode(RELAY, OUTPUT);
  
  sensors.begin();

  for(int i = 0; i < 50; i++)
  {
    sensors.requestTemperatures();
    temperature[i] = sensors.getTempFByIndex(0);
    float aread = analogRead(POT);
    //target[i%5] = map_float(aread, 0, 1023, LOW_END, HIGH_END);
    delay(5);
  }
  
}

void updateAverage()
{
  float sum = 0;
  float sum2 = 0;
  for(int i = 0; i < 50; i++)
  {
    sum += temperature[i];
    //sum2 += target[i%5];
  }

  average = sum / 50;
  //targavg = sum2 / 50;
}

void loop()
{
  sensors.requestTemperatures();
  //lcd.clear();

  temperature[count] = sensors.getTempFByIndex(0);
  
  float aread = analogRead(POT);
  targavg = map_float(aread, 0, 1023, LOW_END, HIGH_END);
  count++;
  if(count==50) count = 0;

  updateAverage();

  if(average > targavg + DELTA_T && !isOn)
  {
    digitalWrite(RELAY, HIGH);
    isOn = true;
  }

  if(average <= targavg - DELTA_T && isOn)
  {
    digitalWrite(RELAY, LOW);
    isOn = false;
  }
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(average);
  lcd.print(" F");
  lcd.setCursor(0,1);
  lcd.print("Target: ");
  lcd.print(targavg);
  lcd.print(" F");
  delay(20);
}
