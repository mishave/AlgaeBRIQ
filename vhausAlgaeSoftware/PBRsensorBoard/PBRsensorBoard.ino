#include <Wire.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial softSerial(8, 7); // RX, TX
float luxPV, phPV, doPV, tempPV, TurbPV, co2InPV, co2OutPV, presPV;
//I2C Slave Address
const int SensorBoardAdd = 8;
const int DOmeterAdd = 97;
const int RTDmeterAdd = 102;
const int PHmeterAdd = 99;

unsigned long currentMillis;
unsigned long previousMillis;
int interval = 5000;

char input[1200];
// I2C Commands
char command = 0;
enum {
  CMD_SB01 = 11,
  CMD_SB02 = 12,
  CMD_SB03 = 13
};

char Atlasdata[20];
float ph_data, do_data, rtd_data;
const int readingDelay = 800;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  softSerial.begin(9600);
}
//////////////////////////////////////////////////////////
//              Handles incoming data                   //

void loop() {

  ph_data = RequestMeterData(PHmeterAdd);
  rtd_data = RequestMeterData(RTDmeterAdd);
  do_data = RequestMeterData(DOmeterAdd);
  updateESP();
  //ReadTempHumidity();
  //ReadLux();
  //ReadCO2();

}

void updateESP() {
  if (softSerial.available()) {
    const size_t capacity = JSON_OBJECT_SIZE(26);
    DynamicJsonDocument doc(capacity);
    doc["luxPV"] = luxPV;
    doc["phPV"] = ph_data;
    doc["doPV"] = do_data;
    doc["tempPV"] = rtd_data;
    doc["TurbPV"] = TurbPV;
    doc["co2InPV"] = co2InPV;
    doc["co2OutPV"] = co2OutPV;
    doc["presPV"] = presPV;
    serializeJson(doc, softSerial);
    Serial.println(ph_data);
  }
}

/////////////////////////////////////////////////////////
//             Request EZO meter readings               //
//////////////////////////////////////////////////////////
float RequestMeterData(const int ezoAdd) {
  //currentMillis = millis();
  //if(currentMillis - previousMillis > interval) {
  float ezoReading;
  previousMillis = currentMillis;
  sendCommand(ezoAdd, 'r', 20);
  delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
  return ezoReading = readMeter(ezoAdd, 20, 1);
  
}

//////////////////////////////////////////////////////////
//              Read EZO meter reading                  //
//////////////////////////////////////////////////////////
float readMeter(const int WireAdd, const int cmd, const int responseSize)  {
  char dataStorage[20];
  byte i = 0;
  Wire.requestFrom(WireAdd, cmd, responseSize);  //call the circuit and request 20 bytes (this may be more than we need)
  int code = Wire.read();

  while (Wire.available()) {
    byte in_char = Wire.read();
    dataStorage[i] = in_char;           //load byte into our array.
    i += 1;                             //next bit.
    if (in_char == 0) {                 //throw null error.
      i = 0;                            //reset the counter i to 0.
      break;                            //exit
    }
  }
  float datafloat = atof(dataStorage);
  return datafloat;
}
//////////////////////////////////////////////////////////
//              Send Request for data                   //
//////////////////////////////////////////////////////////
void sendCommand (const int WireAdd, const byte cmd, const int responseSize)  {
  Wire.beginTransmission (WireAdd);
  Wire.write (cmd);
  Wire.endTransmission ();

  if (WireAdd != PHmeterAdd) {
    Wire.requestFrom (WireAdd, responseSize);
  }
}