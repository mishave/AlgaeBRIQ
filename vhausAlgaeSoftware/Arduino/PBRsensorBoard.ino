#include <Wire.h>
#include <ArduinoJson.h>

const int SensorBoardAdd = 8;
char command=0;
enum {
    CMD_SB01 = 11,
    CMD_SB02 = 12,
    CMD_SB03 = 13
    };

void setup(){
  Wire.begin(SensorBoardAdd);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void receiveEvent (int howMany)
  {
  command = Wire.read ();  // remember command for when we get request
  }


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
  doc1["s1"] = 1200;
  doc1["s2"] = 2200;
  doc1["s3"] = 2300;
  serializeJson(doc1, Wire);
}

void dumpSB02(){  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc2(capacity);
  doc2["s4"] = 5200;
  doc2["s5"] = 5200;
  doc2["s6"] = 5300;
  serializeJson(doc2, Wire);
}

void dumpSB03(){  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc3(capacity);
  doc3["s7"] = 4240;
  doc3["s8"] = 4242;
  doc3["s9"] = 2214;
  serializeJson(doc3, Wire);
}

void loop(){
  
}
