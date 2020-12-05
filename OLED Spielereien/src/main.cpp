#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SSD1306.h>

const int buttonPin = D3;
const int ledPin = BUILTIN_LED;
int buttonState = 0;

Adafruit_BMP085 bmp;
#define OLED_RESET 0 // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

void setup()
{
  Serial.begin(9600);
  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1)
    {
    }
  }

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 64x48)
  // init done
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, buttonState);
}

void displayEnvironment()
{
  display.setCursor(0, 0);
  display.clearDisplay();
  display.setTextSize(2);
  display.print(bmp.readTemperature());
  display.println("");
  display.setTextSize(1);
  display.println("");
  display.println("Altitude: ");
  display.print(bmp.readAltitude());
  display.println(" m");
  display.display();
  delay(2000);
}

void loop()
{
  displayEnvironment();
  digitalWrite(ledPin, digitalRead(buttonPin));
}
