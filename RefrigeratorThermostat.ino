#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DS18S20 is plugged into port 2
#define ONE_WIRE_BUS 2


#define LOW_END 30  //degrees F
#define HIGH_END 45 //degrees F

#define POT A0      // potentiometer for setting target
#define RELAY 10    // relay for the compressor

#define DELTA_T 3   // cycle the compressor at target + or - DELTA_T


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// take the average temperature over the past 50 readings to smooth out any anomalous readings.
float temperature[50];
float average = 0;
int count = 0;

float target = 36;  //target temperature

bool isOn = false;

// because the built in map function returns an int
float map_float(float x, float a, float b, float A, float B)
{
  float m = (B-A)/(b-a);
  float y = (x - a)*m + A;

  return y;
}

// map to target range with 0.25 precision.
float map_25inc(int x, int a, int b, int A, int B)
{
   x = map(x, a, b, A*4, B*4);
   float y = (float)x / 4.0;

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
    delay(5);
  }

  updateAverage();
  
}

// average all the last 50 temperature readings
void updateAverage()
{
  float sum = 0;
  for(int i = 0; i < 50; i++)
  {
    sum += temperature[i];
  }

  average = sum / 50;
}

void loop()
{
  sensors.requestTemperatures();

  temperature[count] = sensors.getTempFByIndex(0);
  count++;
  if(count==50) count = 0;
  
  updateAverage();
  
  int aread = analogRead(POT);
  target = map_25inc(aread, 0, 1023, LOW_END, HIGH_END);
  
  if(average > target + DELTA_T && !isOn)
  {
    digitalWrite(RELAY, HIGH);
    isOn = true;
  }

  if(average <= target - DELTA_T && isOn)
  {
    digitalWrite(RELAY, LOW);
    isOn = false;
  }

  
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(average);
  lcd.print(" F");
  lcd.setCursor(0,1);
  lcd.print("Target: ");
  lcd.print(target);
  lcd.print(" F");
  
  delay(20);
}
