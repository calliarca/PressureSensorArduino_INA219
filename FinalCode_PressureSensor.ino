#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <DHT.h>

Adafruit_INA219 ina219;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DHT_PIN 2     // Digital pin where the DHT22 is connected
#define RELAY_PIN 6  // Digital pin where the relay module is connected

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Define the relay control pin
const int relayPin = 7; // Change this to the pin you've connected the relay to

#define DHT_TYPE DHT22  // Type of DHT sensor (DHT11, DHT22, DHT21)

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
float psiVal;

const int numReadings = 10; // Number of readings to average
float currentReadings[numReadings];
int currentReadingIndex = 0;

void setup(void)
{

  Serial.begin(115200);
  while (!Serial) {
    // will pause Zero, Leonardo, etc until serial console opens
    delay(1);
  }
  uint32_t currentFrequency;

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  // Initialize the relay pin as an OUTPUT
  pinMode(relayPin, OUTPUT);
  // Turn off the relay initially

  digitalWrite(relayPin, LOW);
  // Clear the buffer
  display.clearDisplay();
  Serial.println("Measuring voltage and current with INA219 ...");


}

void loop(void)
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float totalCurrent = 0;
  

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  // Add the current reading to the array
  currentReadings[currentReadingIndex] = current_mA;

  // Increment the reading index
  currentReadingIndex++;

  // If we've reached the end of the array, wrap around to the beginning
  if (currentReadingIndex >= numReadings) {
    currentReadingIndex = 0;
  }

  // Calculate the total current by averaging the readings in the array
  for (int i = 0; i < numReadings; i++) {
    totalCurrent += currentReadings[i];
  }
  current_mA = totalCurrent / numReadings;

  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  if (current_mA < 6.8) {
    // Turn on the relay
    digitalWrite(relayPin, HIGH);
  } else {
    // Turn off the relay
    digitalWrite(relayPin, LOW);
  }
  psiVal = (current_mA - 4)/0.103;
  display.clearDisplay();
  voltCurrent();
  delay(1000);
}

void voltCurrent() {
  display.setFont();
  display.setTextSize(1.5); // Draw 2X-scale text
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Voltage : ");            // Start at top-left corner
  display.print(loadvoltage);
  display.print(" V");

  display.setCursor(0, 10);
  display.print("Current : ");
  display.print(current_mA);
  display.print(" mA");

  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(10, 40);
  display.print(psiVal);
  display.print("psi");
  display.display();
}
