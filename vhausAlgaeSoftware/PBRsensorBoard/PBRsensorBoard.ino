#include <Wire.h>
#include <ArduinoJson.h>

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

void setup(){
  Wire.begin();
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}
//////////////////////////////////////////////////////////
//              Handles incoming data                   //
//////////////////////////////////////////////////////////
void receiveEvent (int howMany)
  {
  command = Wire.read ();  // remember command for when we get request
  }

//////////////////////////////////////////////////////////
//              Handels outgoing data                   //
//////////////////////////////////////////////////////////
void requestEvent ()  {
  switch (command)  {
     case CMD_SB01:      dumpSB01(); break;  // send 1st Packet 
     case CMD_SB02:      dumpSB02(); break;  // send 2nd Packet
     case CMD_SB03:      dumpSB03(); break;  // send 3rd Packet
     }
  }
  
void dumpSB01(){  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc1(capacity);
  doc1["s1"] = ph_data;
  doc1["s2"] = rtd_data;
  doc1["s3"] = do_data;
  serializeJson(doc1, Wire);
}

void dumpSB02(){  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc2(capacity);
  doc2["s4"] = 150;
  doc2["s5"] = 20.6;
  doc2["s6"] = 18.3;
  serializeJson(doc2, Wire);
}

void dumpSB03(){  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc3(capacity);
  doc3["s7"] = 1200;
  doc3["s8"] = 20.3;
  doc3["s9"] = 2214;
  serializeJson(doc3, Wire);
}

void loop(){
  
  RequestMeterData(PHmeterAdd, ph_data);
  
  RequestMeterData(RTDmeterAdd, rtd_data);
  
  RequestMeterData(DOmeterAdd, do_data);
  //ReadTempHumidity();
  //ReadLux();
  //ReadCO2();

}

/////////////////////////////////////////////////////////
//             Request EZO meter readings               //
//////////////////////////////////////////////////////////
float RequestMeterData(const int ezoAdd, float ezoReading) {
  //currentMillis = millis();
  //if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    sendCommand(ezoAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    ezoReading = readMeter(ezoAdd, 20, 1);
  /*
    sendCommand(RTDmeterAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    rtd_data = readMeter(RTDmeterAdd, 20, 1);
  
    sendCommand(DOmeterAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    do_data = readMeter(DOmeterAdd, 20, 1);
    if(ph_data!=0){
    Serial.println(ph_data);              //print the data.
    }

  //}*/
  
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
  
  if (WireAdd!=PHmeterAdd){
    Wire.requestFrom (WireAdd, responseSize);
  }
}
  
//////////////////////////////////////////////////////////
//              Merge JSON messages                     //
/////////////////////////////////////////////////////////
void merge(JsonVariant dst, JsonVariantConst src) {
  //merge(doc.as<JsonVariant>(), doc1.as<JsonVariant>()); //command to use if needed to call
  if (src.is<JsonObject>()) {
    for (auto kvp : src.as<JsonObject>()) {
      merge(dst.getOrAddMember(kvp.key()), kvp.value());
    }
  }
  else {
    dst.set(src);
  }
}

