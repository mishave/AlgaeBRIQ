#include <SparkFun_Qwiic_Humidity_AHT20.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include <SparkFun_Qwiic_Humidity_AHT20.h>
AHT20 humiditySensor;

#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <Wire.h>

#define RXD2 16
#define TXD2 17

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);


const int DOmeterAdd = 97;
const int RTDmeterAdd = 102;
const int PHmeterAdd = 99;

char Atlasdata[20];
const int readingDelay = 800;

float phSV, rtdPV, doSV, tempSV, TurbSV, humSV;
int co2OutSV, wlPV;
float luxSV, irSV, fullSV, visSV;


//Input setup
int turbPin = 34;

unsigned long currentMillis;
unsigned long previousMillis;
int interval = 5000;


// MQTT Network
const char* mqtt_server = "192.168.0.200";

// Home Assistant Credentials
const char *HA_USER = "vhausTech";
const char *HA_PASS = "vhaus";

//Node ID
const char *ID = "sensorESP32";  // Name of our device, must be unique


WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = topic;
  Serial.print("Topic: ");
  Serial.println(topicStr);
  payload[length] = '\0';
  String payloadStr = String((char*)payload);
  Serial.println(payloadStr);

}

// Reconnect to client
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID, HA_USER, HA_PASS)) {
      Serial.println("connected");
      //publish test connection
      client.publish("outTopic", "hello world");
      //subscribe to topics
      //Switches
      //client.subscribe("brainOutSV1");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Wire.begin();

  configureSensor();

  if (humiditySensor.begin() == false)
  {
    while (1);
  }
  
  pinMode(35, INPUT);
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  setup_wifi(); // Connect to network
  delay(1500);
}

void setup_wifi() {
  Serial.print("\nConnecting to ");
  //WiFiManager
  WiFiManager wifiManager;
  Serial.println("started");
  //wifiManager.resetSettings(); // turn off after intital settingfs are done
  wifiManager.setCustomHeadElement("<style>html{filter: invert(40%); -webkit-filter: invert(40%);}</style>");
  WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
  wifiManager.addParameter(&custom_text);
  wifiManager.setAPStaticIPConfig(IPAddress(100, 100, 100, 100), IPAddress(100, 100, 100, 100), IPAddress(255, 255, 255, 0));
  wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
  tsl2591Gain_t gain = tsl.getGain();

}

void loop() {
  phSV = RequestMeterData(PHmeterAdd);
  rtdPV = RequestMeterData(RTDmeterAdd);
  doSV = RequestMeterData(DOmeterAdd);
  readLight();
  if (humiditySensor.available() == true)
  {
    tempSV = humiditySensor.getTemperature();
    humSV = humiditySensor.getHumidity();
  }
  co2OutSV = readCO2UART();

  readInputs();


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  packetUpDate();
}

void packetUpDate() {
  const size_t capacity = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument doc1(capacity);
  DynamicJsonDocument doc2(capacity);
  //DynamicJsonDocument doc3(capacity);
  doc1["phSV"] = phSV;
  doc1["rtdPV"] = rtdPV;
  doc1["doSV"] = doSV;
  doc1["tempSV"] = tempSV;
  doc1["TurbSV"] = TurbSV;
  doc1["luxSV"] = luxSV;
  doc1["co2OutSV"] = co2OutSV;
  doc1["humSV"] = humSV;
  char buffer[256];
  size_t n = serializeJson(doc1, buffer);
  client.publish("sensorOutPV1", buffer, n);
  doc2["luxSV"] = luxSV;
  doc2["irSV"] = irSV;
  doc2["fullSV"] = fullSV;
  doc2["visSV"] = visSV;
  doc2["wlPV"] = wlPV;
  //doc2["co2InPV"] = co2InPV;
  //doc2["co2OutPV"] = co2OutPV;
  //doc2["presPV"] = presPV;
  buffer[256];
  n = serializeJson(doc2, buffer);
  client.publish("sensorOutPV2", buffer, n);
  //doc3["luxSV"] = luxSV;
  //doc3["phSV"] = phSV;
  //doc3["doSV"] = doSV;
  // doc3["tempSV"] = tempSV;
  //doc3["TurbSV"] = TurbSV;
  //doc3["co2InSV"] = co2InSV;
  //doc3["co2OutSV"] = co2OutSV;
  //doc3["presSV"] = presSV;
  // n = serializeJson(doc3, buffer);
  ///client.publish("sensorOutSV1", buffer, n);
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

void readLight() {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  irSV = ir;
  visSV = full;
  fullSV = full - ir;
  luxSV = tsl.calculateLux(full, ir);
}

int readCO2UART() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte response[9]; // for answer

  Serial2.write(cmd, 9); //request PPM CO2

  // clear the buffer
  memset(response, 0, 9);
  int i = 0;
  while (Serial2.available() == 0) {
    i++;
  }
  if (Serial2.available() > 0) {
    Serial2.readBytes(response, 9);
  }
  /*
    for (int i = 0; i < 9; i++) {
    Serial.print(String(response[i], HEX));
    Serial.print("   ");
    }
    Serial.println("");*/
  // ppm
  int ppm_uart = 256 * (int)response[2] + response[3];
  // temp
  byte temp = response[4] - 40;
  return ppm_uart;
}

void readInputs() {
  float turbRaw = ((float)analogRead(turbPin));
  float turbValue = (turbRaw * 3.3) / 4095.0;
  float turbMap = (turbValue - 0) * (5.0 - 0) / (3.3 - 0) + 0;
  if (turbMap < 2.5) {
    TurbSV = 3000;
  }
  else if (turbMap >= 4.2) {
    TurbSV = 0;

  }
  else {
    TurbSV = -1120.4 * sq(turbMap) + 5742.3 * turbMap - 4353.8;
  }
  //char a[100];
  //sprintf(a, "raw: %f  Val: %f  Map: %f  NTU: %f", turbRaw, turbValue, turbMap, TurbSV);
  //Serial.println(a);

  wlPV = digitalRead(35);


}
