#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Thread.h>


// DS18S20 is plugged into port 2
#define ONE_WIRE_BUS 2

#define LOW_END 30  //degrees F
#define HIGH_END 45 //degrees F

#define POT A0      // potentiometer for setting target
#define RELAY 10    // relay for the compressor

#define DELTA_T 3   // cycle the compressor at target + or - DELTA_T


Thread displayCycle = Thread();
int displayIndex = 0;
#define CYCLE_INTERVAL 2000

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

DeviceAddress bottomThermometer = { 0x28, 0xAA, 0x3D, 0x0B, 0x55, 0x14, 0x01, 0xB6 };
DeviceAddress topThermometer   = { 0x28, 0xAA, 0x7D, 0x13, 0x55, 0x14, 0x01, 0x7B };


float topTemperature;
float bottomTemperature;
float avgTemp;
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

  displayCycle.onRun(cycleDisplay);
  displayCycle.setInterval(CYCLE_INTERVAL);

}

void cycleDisplay()
{
  lcd.setCursor(0,0);
  switch(displayIndex)
  {
    case 0:   lcd.print("Top: ");
              lcd.print(topTemperature);
              lcd.print("F      ");
              break;
    case 1:   lcd.print("Bottom: ");
              lcd.print(bottomTemperature);
              lcd.print("F      ");
              break;
    case 2:   lcd.print("Avg: ");
              lcd.print(avgTemp);
              lcd.print("F      ");
              break;
    case 3:   lcd.print("Compressor ");
              if(isOn) lcd.print("On");
              else lcd.print("Off");
              break;
    default:  lcd.print("display error...");
              break;
  }
  displayIndex++;
  if(displayIndex > 3) displayIndex = 0;
}

void loop()
{
  sensors.requestTemperatures();

  topTemperature = sensors.getTempF(topThermometer);
  bottomTemperature = sensors.getTempF(bottomThermometer);
  avgTemp = topTemperature + bottomTemperature;
  avgTemp /= 2;
  
  
  int aread = analogRead(POT);
  target = map_25inc(aread, 0, 1023, LOW_END, HIGH_END);
  
  if(avgTemp > target + DELTA_T && !isOn)
  {
    digitalWrite(RELAY, HIGH);
    isOn = true;
  }

  if(avgTemp <= target - DELTA_T && isOn)
  {
    digitalWrite(RELAY, LOW);
    isOn = false;
  }

  if(displayCycle.shouldRun())
    displayCycle.run();
  
  lcd.setCursor(0,1);
  lcd.print("Target: ");
  lcd.print(target);
  lcd.print(" F");
  
  delay(20);
}
