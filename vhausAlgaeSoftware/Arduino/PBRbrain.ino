#include <Wire.h>
#include <ArduinoJson.h>

const size_t capacity = JSON_OBJECT_SIZE(3)+20;
DynamicJsonDocument doc(300);

//I2C Slave Address
const int SensorBoardAdd = 8;
const int PiAdd = 7;
const int DOmeterAdd = 97;
const int RTDmeterAdd = 102;
const int PHmeterAdd = 99;
char Atlasdata[20];
float ph_data, do_data, rtd_data;
const int readingDelay = 800;


// I2C Commands
char command=0;
enum {
    CMD_Pi01 = 1,
    CMD_Pi02 = 2,
    CMD_Pi03 = 3,
    CMD_Pi04 = 4,
    CMD_Pi05 = 5,
    CMD_Pi06 = 6,
    CMD_Pi07 = 7,
    CMD_Pi08 = 8,
    CMD_Pi09 = 9,
    CMD_Pi010 = 10,
    
    CMD_SB01 = 11,
    CMD_SB02 = 12,
    CMD_SB03 = 13
    };
    

//////////////////////////////////////////////////////////
//              Set up routine - set up                 //
//                and intial conditions                 //
//////////////////////////////////////////////////////////
void setup(){
  Wire.begin(PiAdd);
  Wire.begin(SensorBoardAdd);
  Wire.begin(DOmeterAdd);
  Wire.begin(RTDmeterAdd);
  Wire.begin(PHmeterAdd);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent); 
  Serial.begin(9600);
  Serial.println("setup::done");
}

//////////////////////////////////////////////////////////
//              Handles incoming data                   //
//////////////////////////////////////////////////////////
void receiveEvent (int howMany) {
  command = Wire.read ();  // remember command for when we get request
  }

//////////////////////////////////////////////////////////
//              Handels outgoing data                   //
//////////////////////////////////////////////////////////
void requestEvent ()  {
  switch (command)  {
     //case CMD_SB01:      dumpSB01(); break;  // send 1st Packet 
     //case CMD_SB02:      dumpSB02(); break;  // send 2nd Packet
     //case CMD_SB03:      dumpSB03(); break;  // send 3rd Packet
     }
  
  }

//////////////////////////////////////////////////////////
//              Main program routine, handles           //
//                    general program                   //
//////////////////////////////////////////////////////////
void loop() {
  RequestSensorBoard();
  RequestMeterData();
  delay(50);
}

//////////////////////////////////////////////////////////
//              Read Sensor Board data and              //
//                   combined for use                   //
//////////////////////////////////////////////////////////
void RequestSensorBoard() {
  if (Wire.requestFrom (SensorBoardAdd, capacity) == 0) {
      //Use to throw an error need to think about this ;/
      Serial.println("Sensor Board Error::No reply");
      }
    else  {
      sendCommand (SensorBoardAdd, CMD_SB01, capacity);
      char json[capacity];
      DynamicJsonDocument doc1(capacity);
      deserializeJson(doc1, Wire);
      int sb1 = doc1["s1"]; // 10
      int sb2 = doc1["s2"]; // 11
      int sb3 = doc1["s3"]; // 23
    
      sendCommand (SensorBoardAdd, CMD_SB02, capacity);
      DynamicJsonDocument doc2(capacity);
      deserializeJson(doc2, Wire);
      int sb4 = doc2["s4"]; // 10
      int sb5 = doc2["s5"]; // 11
      int sb6 = doc2["s6"]; // 23
   
      sendCommand (SensorBoardAdd, CMD_SB03, capacity);
      DynamicJsonDocument doc3(capacity);
      deserializeJson(doc3, Wire);
      int sb7 = doc3["s7"]; // 10
      int sb8 = doc3["s8"]; // 11
      int sb9 = doc3["s9"]; // 23
      
      //Combine messages for use
      merge(doc.as<JsonVariant>(), doc1.as<JsonVariant>());
      merge(doc.as<JsonVariant>(), doc2.as<JsonVariant>());
      merge(doc.as<JsonVariant>(), doc3.as<JsonVariant>());
      //serializeJsonPretty(doc, Serial);
    }
 
}

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
 
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 200;           // interval at which to blink (milliseconds)

void RequestMeterData() {
  sendCommand(PHmeterAdd, 'r', 20);
  delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
  ph_data = readMeter(PHmeterAdd, 20, 1);

  sendCommand(RTDmeterAdd, 'r', 20);
  delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
  rtd_data = readMeter(RTDmeterAdd, 20, 1);

  sendCommand(DOmeterAdd, 'r', 20);
  delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
  do_data = readMeter(DOmeterAdd, 20, 1);
  if(ph_data!=0){
  Serial.println(ph_data);              //print the data.
  }
}

float readMeter(const int WireAdd, const int cmd, const int responseSize)  {
  char dataStorage[20];
  byte i = 0;
  Wire.requestFrom(WireAdd, cmd, responseSize);                                           //call the circuit and request 20 bytes (this may be more than we need)
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
  if (src.is<JsonObject>()) {
    for (auto kvp : src.as<JsonObject>()) {
      merge(dst.getOrAddMember(kvp.key()), kvp.value());
    }
  }
  else {
    dst.set(src);
  }
}