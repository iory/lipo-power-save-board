// Mainly copied from
// https://registry.platformio.org/libraries/adafruit/Adafruit%20SGP30%20Sensor/examples/sgp30test/sgp30test.ino
#include <Wire.h>
#include <Adafruit_SGP30.h>

Adafruit_SGP30 sgp;

constexpr int EN_BATT = 11;
constexpr int EN_I2C = 34;
constexpr int EN_VCC33 = 35;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {
  pinMode(EN_BATT, OUTPUT);
  digitalWrite(EN_BATT, HIGH);
  pinMode(EN_I2C, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_I2C, LOW);
  pinMode(EN_VCC33, OUTPUT_OPEN_DRAIN);
  digitalWrite(EN_VCC33, LOW);
  pinMode(21, OUTPUT_OPEN_DRAIN);
  digitalWrite(21, LOW);

  USBSerial.begin(115200);
  while (!USBSerial) { delay(10); } // Wait for serial console to open!
  USBSerial.println("SGP30 test");

  Wire.begin(46, 45); // (SDA, SCL)
  delay(100);
  if (! sgp.begin()){
    USBSerial.println("Sensor not found :(");
    while (1);
  }
  USBSerial.print("Found SGP30 serial #");
  USBSerial.print(sgp.serialnumber[0], HEX);
  USBSerial.print(sgp.serialnumber[1], HEX);
  USBSerial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  // sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}

bool before_first_init_air_quality = true;
int counter = 0;
void loop() {
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  float temperature = 22.1; // [°C]
  float humidity = 45.2; // [%RH]
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  if (! sgp.IAQmeasure()) {
    USBSerial.println("Measurement failed");
    return;
  }
  if (before_first_init_air_quality) {
    String message = "Initializing TVOC and eCO2. Wait for " + String(30 - counter) + "[s]";
    USBSerial.println(message);
  }
  else {
    USBSerial.print("TVOC "); USBSerial.print(sgp.TVOC); USBSerial.print(" ppb\t");
    USBSerial.print("eCO2 "); USBSerial.print(sgp.eCO2); USBSerial.println(" ppm");
  }

  if (! sgp.IAQmeasureRaw()) {
    USBSerial.println("Raw Measurement failed");
    return;
  }
  USBSerial.print("Raw H2 "); USBSerial.print(sgp.rawH2); USBSerial.print(" \t");
  USBSerial.print("Raw Ethanol "); USBSerial.print(sgp.rawEthanol); USBSerial.println("");
  USBSerial.println("");

  delay(1000);

  counter++;
  if (counter == 30) {
    before_first_init_air_quality = false;
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      USBSerial.println("Failed to get baseline readings");
      return;
    }
    USBSerial.print("****Baseline values: eCO2: 0x"); USBSerial.print(eCO2_base, HEX);
    USBSerial.print(" & TVOC: 0x"); USBSerial.println(TVOC_base, HEX);
    USBSerial.println("");
  }
}
