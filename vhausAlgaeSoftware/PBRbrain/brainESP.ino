#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#define RXD2 16
#define TXD2 17

//Auto Cycle
unsigned long updateCurrentMillis = 0;
unsigned long lastUpdateDelay = 0;
unsigned long updateDelay = 1000;

//start and harvest flags
unsigned long startMillis, lastStartDelay = 0;
const long StartDelay = 3000;
int startCycleFlag = 0, startHarvestFlag = 0;

//Check PBR Time
String LastTime = "1", lastLightOn = "1", LastStartHour = "1", LastStartMin = "1";
String pbrTimeStr = " ", pbrLightOnHours = "14", pbrLightStartHour = "7", pbrLightStartMinuet = "0";
int pbrTime = 0, lightStartTime = 420, lightOffTime = 1260;

//Update TImer
byte checkTime = 0;
long countDownFrom = 20160, remainingCycle = 0, remainingCycleLast = 0;
unsigned long cycleCurrentMillis, lastCycleDelay = 0, MinDelay = 5000;
unsigned long setCycleLength = 0;

int checkServo = 0;

int checkAM = 0;

//Water Level Check
unsigned long pbrWaterLowMillis, pbrWaterFullMillis;
unsigned long lastPumpOn = 0, pumpOnDelay = 100;
unsigned long lastPumpOff = 0, pumpOffDelay = 100;
int pumpOnCheck = 0;

//Light cycle check
int LightOnCheck = 0;
unsigned long lightOnMillis, lastLightOnDelay = 0, LightOnDelay = 1000;
unsigned long lightOffMillis, lastLightOffDelay = 0, LightOffDelay = 1000;

//Ph Check
int pHCheck = 0, pumpOn = 0;
unsigned long  pHDowncurrentMillis, lastpHDownDelay, pHDownDelay = 5000;
unsigned long  pHUpcurrentMillis, lastpHUpDelay, pHUpDelay = 5000;
unsigned long  pHOffcurrentMillis, lastpHOffDelay, pHOffDelay = 1000;

//check Temp
int tempCheck = 0;
unsigned long chillcurrentMillis, lastChillDelay, ChillDelay = 5000;
unsigned long heatcurrentMillis, lastHeatDelay, heatDelay = 5000;

//Havest Cycle
int checkHarvest = 0, checkShutOff = 0;

// Status Update
String statusUpdate = " ", alarmUpdate = " ";
unsigned long flashMillis, previousFlashMillis = 0, flashInterval = 150;
int flashState = 0;

// MQTT Network
const char* mqtt_server = "192.168.0.200";

// Home Assistant Credentials
const char *HA_USER = "vhausTech";
const char *HA_PASS = "vhaus";

//Node ID
const char *ID = "brainESP32";  // Name of our device, must be unique

int pbrAM = 0, pbrSS = 0, lightAM = 0,
    lp1 = 0, lp1_1 = 0, lp1_2 = 0, lp1_3 = 0, lp1_4 = 0,
    lp2 = 0, lp2_1 = 0, lp2_2 = 0, lp2_3 = 0, lp2_4 = 0,
    chillAM = 0, chillSS = 0, airAM = 0, airSS = 0,
    doseAM = 0, phUp = 0, phDown = 0, nutMix = 0, samplePump = 0, topUp = 0,
    harvestAM, harbestSS;
int chillOn = 0, heatOn = 0, servoAM = 0;
int pbrPressOpen = 0, pbrPressClose = 180, pbrDump1Open = 0, pbrDump1Close = 180, pbrDump2Open = 0, pbrDump2Close = 180;

int updateCycle, cycleCheck, pbrCycleWeeks, pbrCycleDays, pbrCycleHours, pbrCycleMinuets;

float luxPV, phPV, doPV, tempPV, TurbPV, co2InPV, co2OutPV, presPV;
float luxSV, phSV, doSV, tempSV, TurbSV, co2InSV, co2OutSV, presSV;
float luxSVLast, phSVLast, doSVLast, tempSVLast, TurbSVLast, co2InSVLast, co2OutSVLast, presSVLast;

int servoOn = 0;
int press_valve_sv, dump1_valve_sv, dump2_valve_sv;
int press_valve_svLast, dump1_valve_svLast, dump2_valve_svLast;

StaticJsonDocument<256> sensorOutPV2;
int wlIn = 0;
float co2Out = 100;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = topic;
  Serial.print("Topic: ");
  Serial.println(topicStr);
  payload[length] = '\0';
  String payloadStr = String((char*)payload);
  Serial.println(payloadStr);

  if (topicStr == "pbr/pbrAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/pbrAM/status", "ON"), pbrAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/pbrAM/status", "OFF"), pbrAM = 0;
  }
  if (pbrAM == 1) {
    if (topicStr == "pbr/pbrSS/switch") {
      if (payloadStr == "ON") client.publish("pbr/pbrSS/status", "ON"), pbrSS = 1;
      else if (payloadStr == "OFF") client.publish("pbr/pbrSS/status", "OFF"), pbrSS = 0;
    }
  }

  if (topicStr == "pbr/lightAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/lightAM/status", "ON"), lightAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/lightAM/status", "OFF"), lightAM = 0;
  }

  if (lightAM == 0) {
    if (topicStr == "pbr/lp1/switch") {
      if (payloadStr == "ON") {
        client.publish("pbr/lp1/status", "ON"), lp1 = 1;
        client.publish("pbr/lp1_1/status", "ON"), lp1_1 = 1;
        client.publish("pbr/lp1_2/status", "ON"), lp1_2 = 1;
        client.publish("pbr/lp1_3/status", "ON"), lp1_3 = 1;
        client.publish("pbr/lp1_4/status", "ON"), lp1_4 = 1;
      }
      else if (payloadStr == "OFF") {
        client.publish("pbr/lp1/status", "OFF"), lp1 = 0;
        client.publish("pbr/lp1_1/status", "OFF"), lp1_1 = 0;
        client.publish("pbr/lp1_2/status", "OFF"), lp1_2 = 0;
        client.publish("pbr/lp1_3/status", "OFF"), lp1_3 = 0;
        client.publish("pbr/lp1_4/status", "OFF"), lp1_4 = 0;
      }
    }
    if (topicStr == "pbr/lp1_1/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp1_1/status", "ON"), lp1_1 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp1_1/status", "OFF"), lp1_1 = 0;
    }
    if (topicStr == "pbr/lp1_2/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp1_2/status", "ON"), lp1_2 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp1_2/status", "OFF"), lp1_2 = 0;
    }
    if (topicStr == "pbr/lp1_3/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp1_3/status", "ON"), lp1_3 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp1_3/status", "OFF"), lp1_3 = 0;
    }
    if (topicStr == "pbr/lp1_4/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp1_4/status", "ON"), lp1_4 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp1_4/status", "OFF"), lp1_4 = 0;
    }
    if (topicStr == "pbr/lp2/switch") {
      if (payloadStr == "ON") {
        client.publish("pbr/lp2/status", "ON"), lp2 = 1;
        client.publish("pbr/lp2_1/status", "ON"), lp2_1 = 1;
        client.publish("pbr/lp2_2/status", "ON"), lp2_2 = 1;
        client.publish("pbr/lp2_3/status", "ON"), lp2_3 = 1;
        client.publish("pbr/lp2_4/status", "ON"), lp2_4 = 1;
      }
      else if (payloadStr == "OFF") {
        client.publish("pbr/lp2/status", "OFF"), lp2 = 0;
        client.publish("pbr/lp2_1/status", "OFF"), lp2_1 = 0;
        client.publish("pbr/lp2_2/status", "OFF"), lp2_2 = 0;
        client.publish("pbr/lp2_3/status", "OFF"), lp2_3 = 0;
        client.publish("pbr/lp2_4/status", "OFF"), lp2_4 = 0;
      }
    }
    if (topicStr == "pbr/lp2_1/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp2_1/status", "ON"), lp2_1 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp2_1/status", "OFF"), lp2_1 = 0;
    }
    if (topicStr == "pbr/lp2_2/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp2_2/status", "ON"), lp2_2 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp2_2/status", "OFF"), lp2_2 = 0;
    }
    if (topicStr == "pbr/lp2_3/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp2_3/status", "ON"), lp2_3 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp2_3/status", "OFF"), lp2_3 = 0;
    }
    if (topicStr == "pbr/lp2_4/switch") {
      if (payloadStr == "ON") client.publish("pbr/lp2_4/status", "ON"), lp2_4 = 1;
      else if (payloadStr == "OFF") client.publish("pbr/lp2_4/status", "OFF"), lp2_4 = 0;
    }
  }

  if (topicStr == "pbr/chillAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/chillAM/status", "ON"), chillAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/chillAM/status", "OFF"), chillAM = 0;
  }
  if (chillAM == 0) {
    if (topicStr == "pbr/chillSS/switch") {
      if (payloadStr == "ON") client.publish("pbr/chillSS/status", "ON"), chillSS = 1;
      else if (payloadStr == "OFF") client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
    }
  }
  if (topicStr == "pbr/airAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/airAM/status", "ON"), airAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/airAM/status", "OFF"), airAM = 0;
  }
  if (topicStr == "pbr/airSS/switch") {
    if (startCycleFlag == 1 && airAM == 0)  {
      if (payloadStr == "ON") client.publish("pbr/airSS/status", "ON"), airSS = 1;
      else if (payloadStr == "OFF") client.publish("pbr/airSS/status", "OFF"), airSS = 0;
    }
    else if (startCycleFlag == 0  && airAM == 0) {
      if (payloadStr == "ON") client.publish("pbr/airSS/status", "ON"), airSS = 1;
      else if (payloadStr == "OFF") client.publish("pbr/airSS/status", "OFF"), airSS = 0;
    }
  }

  if (topicStr == "pbr/doseAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/doseAM/status", "ON"), doseAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/doseAM/status", "OFF"), doseAM = 0;
  }
  if (topicStr == "pbr/phUp/switch") {
    if (payloadStr == "ON") client.publish("pbr/phUp/status", "ON"), phUp = 1;
    else if (payloadStr == "OFF") client.publish("pbr/phUp/status", "OFF"), phUp = 0;
  }
  if (topicStr == "pbr/phDown/switch") {
    if (payloadStr == "ON") client.publish("pbr/phDown/status", "ON"), phDown = 1;
    else if (payloadStr == "OFF") client.publish("pbr/phDown/status", "OFF"), phDown = 0;
  }
  if (topicStr == "pbr/nutMix/switch") {
    if (payloadStr == "ON") client.publish("pbr/nutMix/status", "ON"), nutMix = 1;
    else if (payloadStr == "OFF") client.publish("pbr/nutMix/status", "OFF"), nutMix = 0;
  }
  if (topicStr == "pbr/samplePump/switch") {
    if (payloadStr == "ON") client.publish("pbr/samplePump/status", "ON"), samplePump = 1;
    else if (payloadStr == "OFF") client.publish("pbr/samplePump/status", "OFF"), samplePump = 0;
  }

  if (topicStr == "pbr/topUp/switch") {
    if (startCycleFlag == 1 && doseAM == 0)  {
      if (payloadStr == "ON") client.publish("pbr/topUp/status", "ON"), topUp = 1;
      else if (payloadStr == "OFF") client.publish("pbr/topUp/status", "OFF"), topUp = 0;
    }
    else if (startCycleFlag == 0  && airAM == 0) {
      if (payloadStr == "ON") client.publish("pbr/topUp/status", "ON"), topUp = 1;
      else if (payloadStr == "OFF") client.publish("pbr/topUp/status", "OFF"), topUp = 0;
    }
  }
  if (topicStr == "pbr/harvestAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/harvestAM/status", "ON"), harvestAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/harvestAM/status", "OFF"), harvestAM = 0;
  }
  if (topicStr == "pbr/harbestSS/switch") {
    if (payloadStr == "ON") client.publish("pbr/harbestSS/status", "ON"), harbestSS = 1;
    else if (payloadStr == "OFF") client.publish("pbr/harbestSS/status", "OFF"), harbestSS = 0;
  }
  //Cycle Length Settings
  if (topicStr == "pbrCycleWeeks") pbrCycleWeeks = payloadStr.toInt();
  if (topicStr == "pbrCycleDays") pbrCycleDays = payloadStr.toInt();
  if (topicStr == "pbrCycleHours") pbrCycleHours = payloadStr.toInt();
  if (topicStr == "pbrCycleMinuets") pbrCycleMinuets = payloadStr.toInt();

  if (topicStr == "luxSV") luxSV = payloadStr.toFloat();
  if (topicStr == "phSV") phSV = payloadStr.toFloat();
  if (topicStr == "doSV") doSV = payloadStr.toFloat();
  if (topicStr == "tempSV") tempSV = payloadStr.toFloat();
  if (topicStr == "TurbSV") TurbSV = payloadStr.toFloat();
  if (topicStr == "co2InSV") co2InSV = payloadStr.toFloat();
  if (topicStr == "co2OutSV") co2OutSV = payloadStr.toFloat();
  if (topicStr == "presSV") presSV = payloadStr.toFloat();

  if (topicStr == "pbr/servoAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/servoAM/status", "ON"), servoAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/servoAM/status", "OFF"), servoAM = 0;
  }
  if (servoAM == 0) {
    if (topicStr == "pbrPressOpen") pbrPressOpen = payloadStr.toInt();
    if (topicStr == "pbrPressOpen") pbrPressOpen = payloadStr.toInt();
    if (topicStr == "pbrPressClose") pbrPressClose = payloadStr.toInt();
    if (topicStr == "pbrDump1Open") pbrDump1Open = payloadStr.toInt();
    if (topicStr == "pbrDump1Close") pbrDump1Close = payloadStr.toInt();
    if (topicStr == "pbrDump2Open") pbrDump2Open = payloadStr.toInt();
    if (topicStr == "pbrDump2Close") pbrDump2Close = payloadStr.toInt();
  }
  if (topicStr == "press_valve_sv") press_valve_sv = payloadStr.toInt();
  if (topicStr == "dump1_valve_sv") dump1_valve_sv = payloadStr.toInt();
  if (topicStr == "dump2_valve_sv") dump2_valve_sv = payloadStr.toInt();

  //Read Sensor Board Data
  if (topicStr == "sensorOutPV2") {
    deserializeJson(sensorOutPV2, payload, length);
    wlIn = sensorOutPV2["wlIn"];
    co2Out = sensorOutPV2["co2Out"];
  }
  if (topicStr == "pbrTime") pbrTimeStr = payloadStr;
  if (topicStr == "pbrLightOnHours") pbrLightOnHours = payloadStr;
  if (topicStr == "pbrLightStartHour") pbrLightStartHour = payloadStr;
  if (topicStr == "pbrLightStartMinuet") pbrLightStartMinuet = payloadStr;

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
      client.subscribe("pbr/pbrAM/switch");
      client.subscribe("pbr/pbrSS/switch");

      client.subscribe("pbr/lightAM/switch");
      client.subscribe("pbr/lp1/switch");
      client.subscribe("pbr/lp1_1/switch");
      client.subscribe("pbr/lp1_2/switch");
      client.subscribe("pbr/lp1_3/switch");
      client.subscribe("pbr/lp1_4/switch");
      client.subscribe("pbr/lp2/switch");
      client.subscribe("pbr/lp2_1/switch");
      client.subscribe("pbr/lp2_2/switch");
      client.subscribe("pbr/lp2_3/switch");
      client.subscribe("pbr/lp2_4/switch");

      client.subscribe("pbr/chillAM/switch");
      client.subscribe("pbr/chillSS/switch");

      client.subscribe("pbr/airAM/switch");
      client.subscribe("pbr/airSS/switch");

      client.subscribe("pbr/doseAM/switch");
      client.subscribe("pbr/phUp/switch");
      client.subscribe("pbr/phDown/switch");
      client.subscribe("pbr/nutMix/switch");
      client.subscribe("pbr/samplePump/switch");
      client.subscribe("pbr/topUp/switch");

      client.subscribe("pbr/harvestAM/switch");
      client.subscribe("pbr/harbestSS/switch");
      client.subscribe("pbr/coolingSS/switch");
      client.subscribe("pbr/heatingSS/switch");


      client.subscribe("pbr/servoAM/switch");
      client.subscribe("pbrPressOpen");
      client.subscribe("pbrPressClose");
      client.subscribe("pbrDump1Open");
      client.subscribe("pbrDump1Close");
      client.subscribe("pbrDump2Open");
      client.subscribe("pbrDump2Close");

      //Cycle Length Time
      client.subscribe("pbrCycleWeeks");
      client.subscribe("pbrCycleDays");
      client.subscribe("pbrCycleHours");
      client.subscribe("pbrCycleMinuets");

      //Cycle Length Time
      client.subscribe("luxSV");
      client.subscribe("phSV");
      client.subscribe("doSV");
      client.subscribe("tempSV");
      client.subscribe("TurbSV");
      client.subscribe("co2InSV");
      client.subscribe("co2OutSV");
      client.subscribe("presSV");

      //Update ServoSV's
      client.subscribe("press_valve_sv");
      client.subscribe("dump1_valve_sv");
      client.subscribe("dump2_valve_sv");

      //ReadSensorBoard Data
      client.subscribe("sensorOutPV2");


      //ReadSensorBoard Data
      client.subscribe("pbrLightOnHours");
      client.subscribe("pbrLightStartHour");
      client.subscribe("pbrLightStartMinuet");
      client.subscribe("pbrTime");

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
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  setup_wifi(); // Connect to network

  updateCycle = pbrCycleWeeks + pbrCycleDays + pbrCycleHours + pbrCycleMinuets;
  intialiseObjects();
  delay(1500);
}

void setup_wifi() {
  Serial.print("\nConnecting to ");
  //WiFiManager
  WiFiManager wifiManager;
  Serial.println("started");
  //wifiManager.resetSettings(); // turn off after intital settings are done
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

void intialiseObjects() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("pbr/pbrAM/status", "OFF"), pbrAM = 0;;
  client.publish("pbr/pbrSS/status", "OFF"), pbrSS = 0;

  client.publish("pbr/lightAM/status", "OFF"), lightAM = 0;
  client.publish("pbr/lp1/status", "OFF"), lp1 = 0;
  client.publish("pbr/lp1_1/status", "OFF"), lp1_1 = 0;
  client.publish("pbr/lp1_2/status", "OFF"), lp1_2 = 0;
  client.publish("pbr/lp1_3/status", "OFF"), lp1_3 = 0;
  client.publish("pbr/lp1_4/status", "OFF"), lp1_4 = 0;
  client.publish("pbr/lp2/status", "OFF"), lp2 = 0;
  client.publish("pbr/lp2_1/status", "OFF"), lp2_1 = 0;
  client.publish("pbr/lp2_2/status", "OFF"), lp2_2 = 0;
  client.publish("pbr/lp2_3/status", "OFF"), lp2_3 = 0;
  client.publish("pbr/lp2_4/status", "OFF"), lp2_4 = 0;

  client.publish("pbr/chillAM/status", "OFF"), chillAM = 0;
  client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
  client.publish("pbr/chillOn/status", "OFF"), chillOn = 0;
  client.publish("pbr/heatOn/status", "OFF"), heatOn = 0;

  client.publish("pbr/airAM/status", "OFF"), airAM = 0;
  client.publish("pbr/airSS/status", "OFF"), airSS = 0;

  client.publish("pbr/doseAM/status", "OFF"), doseAM = 0;
  client.publish("pbr/phUp/status", "OFF"), phUp = 0;
  client.publish("pbr/phDown/status", "OFF"), phDown = 0;
  client.publish("pbr/nutMix/status", "OFF"), nutMix = 0;
  client.publish("pbr/samplePump/status", "OFF"), samplePump = 0;
  client.publish("pbr/topUp/status", "OFF"), topUp = 0;

  client.publish("pbr/harvestAM/status", "OFF"), harvestAM = 0;
  client.publish("pbr/harbestSS/status", "OFF"), harbestSS = 0;

}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateCurrentMillis = millis(); //send update every x seconds
  if (updateCurrentMillis - lastUpdateDelay >= updateDelay) {
    lastUpdateDelay = updateCurrentMillis;
    packetUpDate();  //dump data
  }
  getPBRTime();
  updateTimers();
  updateInputNumbers();
  updateServos();
  upDateBrain();
  autoCycle();

}

void getPBRTime() {
  //get time from Home assistant
  if (LastTime != pbrTimeStr) {
    String prbHourStr = splitHrMin(pbrTimeStr, ':', 0);
    String pbrMinStr = splitHrMin(pbrTimeStr, ':', 1);
    int prbHour = prbHourStr.toInt();
    int pbrMin = pbrMinStr.toInt();
    pbrTime = prbHour * 60;
    pbrTime = pbrTime + pbrMin;
    LastTime = pbrTimeStr;
  }
  //get light cycle length from home assistant
  if (LastStartHour != pbrLightStartHour ||
      LastStartMin != pbrLightStartMinuet ||
      lastLightOn != pbrLightOnHours) {

    int prbStartHour = pbrLightStartHour.toInt();
    int pbrStartMin = pbrLightStartMinuet.toInt();

    lightStartTime = prbStartHour * 60;
    lightStartTime = lightStartTime + pbrStartMin;

    LastStartHour = pbrLightStartHour;
    LastStartMin = pbrLightStartMinuet;

    int lightOnTime = pbrLightOnHours.toInt();
    lightOffTime = lightOnTime * 60;
    lightOffTime = lightOffTime + lightStartTime;

    lastLightOn = pbrLightOnHours;
  }
}

String splitHrMin(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void upDateBrain()  {
  if (Serial2.available()) {
    const size_t brainCap = JSON_OBJECT_SIZE(27);
    DynamicJsonDocument brain(brainCap);
    brain["lp1_1"] = lp1_1;
    brain["lp1_2"] = lp1_1;
    brain["lp1_3"] = lp1_1;
    brain["lp1_4"] = lp1_1;
    brain["lp2_1"] = lp2_1;
    brain["lp2_2"] = lp2_1;
    brain["lp2_3"] = lp2_1;
    brain["lp2_4"] = lp2_1;
    
    brain["chillOn"] = chillOn;
    brain["heatOn"] = heatOn;

    brain["airSS"] = airSS;

    brain["topUp"] = topUp;
    brain["phDown"] = phDown;
    brain["nutMix"] = nutMix;
    brain["samplePump"] = samplePump;

    brain["harvestAM"] = harvestAM;
    brain["harbestSS"] = harbestSS;

    brain["servoOn"] = servoOn;
    brain["press_valve_sv"] = press_valve_sv;
    brain["dump1_valve_sv"] = dump1_valve_sv;
    brain["dump2_valve_sv"] = dump2_valve_sv;
    
    brain["pbrPressOpen"] = pbrPressOpen;
    brain["pbrPressClose"] = pbrPressClose;
    brain["pbrDump1Open"] = pbrDump1Open;
    brain["pbrDump1Close"] = pbrDump1Close;
    brain["pbrDump2Open"] = pbrDump2Open;
    brain["pbrDump2Close"] = pbrDump2Close;

    serializeJson(brain, Serial2);
    Serial2.println();
    Serial2.println();
  }
}

void updateTimers() {
  updateCycle = pbrCycleWeeks + pbrCycleDays + pbrCycleHours + pbrCycleMinuets;
  if (updateCycle != cycleCheck) {
    updateCycle = pbrCycleWeeks * 10080;
    updateCycle = updateCycle + pbrCycleDays * 1440;
    updateCycle = updateCycle + pbrCycleHours * 60;
    updateCycle = updateCycle + pbrCycleMinuets;
    setCycleLength = updateCycle;
    String str = String(updateCycle);
    const size_t capacityTime = JSON_OBJECT_SIZE(26);
    DynamicJsonDocument cycleTime(capacityTime);
    cycleTime["cycleTime"] = str;
    char buffer[256];
    size_t n = serializeJson(cycleTime, buffer);
    client.publish("pbrCycleTime", buffer, n);
    cycleCheck = pbrCycleWeeks + pbrCycleDays + pbrCycleHours + pbrCycleMinuets;
  }
}

void updateInputNumbers() {
  luxSVLast = sendValFloat("luxSV", luxSV, luxSVLast);
  phSVLast = sendValFloat("phSV", phSV, phSVLast);
  doSVLast = sendValFloat("doSV", doSV, doSVLast);
  tempSVLast = sendValFloat("tempSV", tempSV, tempSVLast);
  TurbSVLast = sendValFloat("TurbSV", TurbSV, TurbSVLast);
  co2InSVLast = sendValFloat("co2InSV", co2InSV, co2InSVLast);
  co2OutSVLast = sendValFloat("co2OutSV", co2OutSV, co2OutSVLast);
  presSVLast = sendValFloat("presSV", presSV, presSVLast);
}

void updateServos() {
  if (press_valve_sv != press_valve_svLast) {
    sendValInt("press_valve_sv", press_valve_sv);
    press_valve_svLast = press_valve_sv;
  }
  if (dump1_valve_sv != dump1_valve_svLast) {
    sendValInt("dump1_valve_sv", dump1_valve_sv);
    dump1_valve_svLast = dump1_valve_sv;
  }
  if (dump2_valve_sv != dump2_valve_svLast) {
    sendValInt("dump2_valve_sv", dump2_valve_sv);
    dump2_valve_svLast = dump2_valve_sv;
  }
}

//////////////////////////////////////////////////////////
//             Handles Float/Int values                 //
//////////////////////////////////////////////////////////
float sendValFloat(const char *topic, float incomingVal, float lastVal) {
  if (lastVal != incomingVal) {
    String str = String(incomingVal);
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    client.publish(topic, char_array, str_len);
    return incomingVal;
  }
}
void sendValInt(const char *topic, int incomingValInt) {
  String str = String(incomingValInt);
  int str_len = str.length() + 1;
  char char_array[str_len];
  str.toCharArray(char_array, str_len);
  client.publish(topic, char_array, str_len);
}

//////////////////////////////////////////////////////////
//              Update Brain Bulk Data                  //
//////////////////////////////////////////////////////////
void packetUpDate() {
  const size_t capacity = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument doc1(capacity);
  DynamicJsonDocument doc2(capacity);
  DynamicJsonDocument doc3(capacity);

  doc1["holdingPV"] = luxPV;
  char buffer[256];
  size_t n = serializeJson(doc1, buffer);
  client.publish("brainOutPV1", buffer, n);

  doc2["holdingSV"] = luxSV;
  n = serializeJson(doc2, buffer);
  client.publish("brainOutSV1", buffer, n);

  doc3["status"] = statusUpdate;
  doc3["alarm"] = alarmUpdate;
  n = serializeJson(doc3, buffer);
  client.publish("brainStatus", buffer, n);
}

//////////////////////////////////////////////////////////
//                 Auto Cycle Routine                   //
//////////////////////////////////////////////////////////
void autoCycle()  {
  if (pbrAM == 1 && pbrSS == 1) {
    startMillis = millis();
    if (startMillis - lastStartDelay >= StartDelay && startCycleFlag == 0) {
      // save the last time you blinked the LED
      lastStartDelay = startMillis;
      startCycleFlag = 1;
    }
  }
  else  {
    startCycleFlag = 0;
    startHarvestFlag = 0;
    remainingCycle = 0;
    checkAM = 0;
    pumpOnCheck = 0;
    LightOnCheck = 0;
    checkServo = 0;
    pHCheck = 0;
    pumpOn = 0;
    tempCheck = 0;
    checkHarvest = 0;
    checkShutOff = 0;

  }

  if (startCycleFlag == 1 && startHarvestFlag == 0)  {
    checkTimeRemaining(); //Check Time Remaining and Update
    setServosClosed();    //Set Dump Valves Are Closed
    checkAir();           //Check That Air Is On
    checkWaterLevel();    //Check Water Level and Top Up
    checkLighting();      //Check Light Status
    checkPh();            //Check pH Level
    checkTemp();          //Check Tempreture
    reportStatus();

  }
  if (startCycleFlag == 2 && startHarvestFlag == 1)  {
    checkTimeRemaining();
    shutOffAuto();
    openCultureValves();
    openPressureValve();
    startHarvestPump();
    reportStatus();
  }

  if (startCycleFlag == 0)  {
    checkTimeRemaining();
    reportStatus();
  }
}

//////////////////////////////////////////////////////////
//             Check Cycle Time and Update              //
//////////////////////////////////////////////////////////
void checkTimeRemaining()   {
  //check time at start of cycle
  if (startCycleFlag == 1 || startHarvestFlag == 1) {
    if (checkTime == 1) {
      remainingCycle = setCycleLength;
      setCycleRemaining(setCycleLength);
      checkTime = 0;
    }
    cycleCurrentMillis = millis();
    if (cycleCurrentMillis - lastCycleDelay >= MinDelay) {
      lastCycleDelay = cycleCurrentMillis;
      if (startHarvestFlag == 0)  {
        remainingCycle = --remainingCycle;
        if (remainingCycle == -1)  {
          setCycleRemaining(0);
          startHarvestFlag = 1;
          startCycleFlag = 2;
        }
        else setCycleRemaining(remainingCycle);
      }
      if (startHarvestFlag == 1)  {
        remainingCycle = ++remainingCycle;
        setCycleRemaining(remainingCycle);

      }
    }
  }
  if (startCycleFlag == 0) {
    if (checkTime == 0) {
      remainingCycle = 0;
      setCycleRemaining(remainingCycle);
      checkTime = 1;
    }
  }
}

void setCycleRemaining(unsigned long cycleLength) {
  const size_t CycleRemaining = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument cycleRemaining(CycleRemaining);
  cycleRemaining["cycleLeft"] = cycleLength;
  char buffer[256];
  size_t n = serializeJson(cycleRemaining, buffer);
  client.publish("pbrCycleLeft", buffer, n);
}


void setServosClosed()  {
  if (startCycleFlag == 1 && checkServo == 0) {
    dump1_valve_sv = 0;
    sendValInt("dump1_valve_sv", dump1_valve_sv);
    dump1_valve_svLast = dump1_valve_sv;

    dump2_valve_sv = 0;
    sendValInt("dump2_valve_sv", dump2_valve_sv);
    dump2_valve_svLast = dump2_valve_sv;

    checkServo = 1;
  }
}

void checkAir() {
  if (startCycleFlag == 1 && checkAM == 0) {
    client.publish("pbr/airAM/status", "ON"), airAM = 1;
    client.publish("pbr/airSS/status", "ON"), airSS = 1;
    checkAM = 1;
  }

}

void checkWaterLevel()  {
  if (startCycleFlag == 1 && pumpOnCheck == 0) {
    client.publish("pbr/doseAM/status", "ON"), doseAM = 1;
    client.publish("pbr/topUp/status", "OFF"), topUp = 0;
    pumpOnCheck = 1;
  }
  else if (startCycleFlag == 1 && pumpOnCheck == 1) {
    if (wlIn == 0) {
      pbrWaterLowMillis = millis();
      if (pbrWaterLowMillis - lastPumpOn >= pumpOnDelay) {
        lastPumpOn = pbrWaterLowMillis;
        topUp = 1;
        if (pumpOnCheck == 1) {
          client.publish("pbr/topUp/status", "ON");
          pumpOnCheck = 2;
        }
      }
    }
    else if (wlIn == 1) {
      pbrWaterFullMillis = millis();
      if (pbrWaterFullMillis - lastPumpOff >= pumpOffDelay) {
        lastPumpOff = pbrWaterFullMillis;
        topUp = 0;
        if (pumpOnCheck == 2) {
          client.publish("pbr/topUp/status", "OFF");
          pumpOnCheck = 1;
        }
      }
    }
  }
}

void checkLighting()  {
  if (startCycleFlag == 1 && LightOnCheck == 0 || LightOnCheck == 4) {
    client.publish("pbr/lightAM/status", "ON"), lightAM = 1;
    if (LightOnCheck == 0) {
      client.publish("pbr/lp1/status", "OFF");
      client.publish("pbr/lp1_1/status", "OFF");
      client.publish("pbr/lp1_2/status", "OFF");
      client.publish("pbr/lp1_3/status", "OFF");
      client.publish("pbr/lp1_4/status", "OFF");

      client.publish("pbr/lp2/status", "OFF");
      client.publish("pbr/lp2_1/status", "OFF");
      client.publish("pbr/lp2_2/status", "OFF");
      client.publish("pbr/lp2_3/status", "OFF");
      client.publish("pbr/lp2_4/status", "OFF");
      lp1 = 0, lp1_1 = 0, lp1_2 = 0, lp1_3 = 0, lp1_4 = 0,
      lp2 = 0, lp2_1 = 0, lp2_2 = 0, lp2_3 = 0, lp2_4 = 0;
    }
    if (pbrTime >= lightStartTime && pbrTime <= lightOffTime) {
      LightOnCheck = 1;
    }
    else LightOnCheck = 2;
  }
  if (pbrTime >= lightStartTime && pbrTime <= lightOffTime && LightOnCheck == 1) {
    client.publish("pbr/lp1/status", "ON");
    client.publish("pbr/lp2/status", "ON");
    client.publish("pbr/lp1_1/status", "ON");
    client.publish("pbr/lp1_2/status", "ON");
    client.publish("pbr/lp1_3/status", "ON");
    client.publish("pbr/lp1_4/status", "ON");
    client.publish("pbr/lp2_1/status", "ON");
    client.publish("pbr/lp2_2/status", "ON");
    client.publish("pbr/lp2_3/status", "ON");
    client.publish("pbr/lp2_4/status", "ON");
    lp1 = 1, lp1_1 = 1, lp1_2 = 1, lp1_3 = 1, lp1_4 = 1,
    lp2 = 1, lp2_1 = 1, lp2_2 = 1, lp2_3 = 1, lp2_4 = 1;
    LightOnCheck = 2;
  }
  else if (pbrTime < lightStartTime || pbrTime > lightOffTime && LightOnCheck == 2) {
    client.publish("pbr/lp1/status", "OFF");
    client.publish("pbr/lp2/status", "OFF");
    client.publish("pbr/lp1_1/status", "OFF");
    client.publish("pbr/lp1_2/status", "OFF");
    client.publish("pbr/lp1_3/status", "OFF");
    client.publish("pbr/lp1_4/status", "OFF");
    client.publish("pbr/lp2_1/status", "OFF");
    client.publish("pbr/lp2_2/status", "OFF");
    client.publish("pbr/lp2_3/status", "OFF");
    client.publish("pbr/lp2_4/status", "OFF");
    lp1 = 0, lp1_1 = 0, lp1_2 = 0, lp1_3 = 0, lp1_4 = 0,
    lp2 = 0, lp2_1 = 0, lp2_2 = 0, lp2_3 = 0, lp2_4 = 0;
    LightOnCheck = 1;
  }

  else if (startCycleFlag == 1 && lightAM == 0 && LightOnCheck != 3) {
    LightOnCheck = 3;
  }
  else if (startCycleFlag == 1 && lightAM == 1 && LightOnCheck == 3) {
    LightOnCheck = 4;
  }
}

void checkPh()  {
  if (startCycleFlag == 1 && pHCheck == 0 || pHCheck == 4) {
    client.publish("pbr/doseAM/status", "ON"), doseAM = 1;
    client.publish("pbr/phUp/status", "OFF"), phUp = 0;
    client.publish("pbr/phDown/status", "OFF"), phDown = 0;
    pHCheck = 1;
  }
  if (startCycleFlag == 1 && pHCheck == 1) {
    //int reading = ph_pv_in;
    if (phPV >= (phSV + 1)) {
      pHDowncurrentMillis = millis();
      if (pHDowncurrentMillis - lastpHDownDelay >= pHDownDelay) {
        lastpHDownDelay = pHDowncurrentMillis;
        client.publish("pbr/phDown/status", "ON"), phDown = 1;
        pumpOn = 1;
        pHCheck = 2;
      }
    }

    if (phPV <= (phSV - 1)) {
      pHUpcurrentMillis = millis();
      if (pHUpcurrentMillis - lastpHUpDelay >= pHUpDelay) {
        lastPumpOff = pHUpcurrentMillis;
        client.publish("pbr/phUp/status", "ON"), phUp = 1;
        pumpOn = 1;
        pHCheck = 2;
      }
    }
  }
}

void checkTemp()  {
  if (startCycleFlag == 1 && tempCheck == 0 || tempCheck == 4) {
    client.publish("pbr/chillAM/status", "ON"), chillAM = 1;
    client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
    client.publish("pbr/chillOn/status", "OFF"), chillOn = 0;
    client.publish("pbr/heatOn/status", "OFF"), heatOn = 0;
    tempCheck = 1;
  }
  //Chiller On/Off
  if (startCycleFlag == 1 && tempCheck == 1) {
    //int reading = ph_pv_in;
    if (tempSV >= (tempPV + 3)) {
      chillcurrentMillis = millis();
      if (chillcurrentMillis - lastChillDelay >= ChillDelay) {
        lastChillDelay = chillcurrentMillis;
        client.publish("pbr/chillSS/status", "ON"), chillSS = 1;
        client.publish("pbr/chillOn/status", "ON"), chillOn = 1;
        tempCheck = 2;
      }
    }
  }
  if (tempSV <= tempPV && chillOn == 1)  {
    client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
    client.publish("pbr/chillOn/status", "OFF"), chillOn = 0;
    tempCheck = 1;
  }
  //Heater On/Off
  if (startCycleFlag == 1 && tempCheck == 1) {
    //int reading = ph_pv_in;
    if (tempSV <= (tempPV + 2)) {
      heatcurrentMillis = millis();
      if (heatcurrentMillis - lastHeatDelay >= heatDelay) {
        lastHeatDelay = heatcurrentMillis;
        client.publish("pbr/chillSS/status", "ON"), chillSS = 1;
        client.publish("pbr/heatOn/status", "ON"), heatOn = 1;
        tempCheck = 2;
      }
    }
  }
  if (tempSV >= tempPV && heatOn == 1)  {
    client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
    client.publish("pbr/heatOn/status", "OFF"), heatOn = 0;
    tempCheck = 1;
  }
  if (startCycleFlag == 1 && chillAM == 0 && tempCheck != 3) {
    tempCheck = 3;
  }
  else if (startCycleFlag == 1 && chillAM == 1 && tempCheck == 3) {
    tempCheck = 4;
  }
}

void shutOffAuto()  {
  if (startHarvestFlag == 1 && checkShutOff == 0) {
    client.publish("pbr/airSS/status", "OFF"), airSS = 0;

    client.publish("pbr/topUp/status", "OFF"), topUp = 0;

    client.publish("pbr/lp1/status", "OFF");
    client.publish("pbr/lp2/status", "OFF");
    client.publish("pbr/lp1_1/status", "OFF");
    client.publish("pbr/lp1_2/status", "OFF");
    client.publish("pbr/lp1_3/status", "OFF");
    client.publish("pbr/lp1_4/status", "OFF");
    client.publish("pbr/lp2_1/status", "OFF");
    client.publish("pbr/lp2_2/status", "OFF");
    client.publish("pbr/lp2_3/status", "OFF");
    client.publish("pbr/lp2_4/status", "OFF");
    lp1 = 0, lp1_1 = 0, lp1_2 = 0, lp1_3 = 0, lp1_4 = 0,
    lp2 = 0, lp2_1 = 0, lp2_2 = 0, lp2_3 = 0, lp2_4 = 0;

    client.publish("pbr/phUp/status", "OFF"), phUp = 0;
    client.publish("pbr/phDown/status", "OFF"), phDown = 0;

    client.publish("pbr/chillOn/status", "OFF"), chillOn = 0;
    client.publish("pbr/heatOn/status", "OFF"), heatOn = 0;
    checkShutOff = 1;
  }
}

void openCultureValves() {
  if (startHarvestFlag == 1 && checkServo == 1) {
    dump1_valve_sv = 100;
    sendValInt("dump1_valve_sv", dump1_valve_sv);
    dump1_valve_svLast = dump1_valve_sv;

    dump2_valve_sv = 100;
    sendValInt("dump2_valve_sv", dump2_valve_sv);
    dump2_valve_svLast = dump2_valve_sv;

    checkServo = 2;
  }
}

void openPressureValve()  {
  if (startHarvestFlag == 1 && checkServo == 2) {
    press_valve_sv = 100;
    sendValInt("press_valve_sv", press_valve_sv);
    press_valve_svLast = press_valve_sv;

    checkServo = 3;
  }
}

void startHarvestPump() {

  if (startHarvestFlag == 1 && checkHarvest == 0) {
    client.publish("pbr/harbestSS/status", "ON"), harbestSS = 1;
    checkHarvest = 1;
  }
}

void reportStatus()  {

  if (pbrAM == 1 && pbrSS == 0) {
    statusUpdate = "System Ready";
  }
  if (startCycleFlag == 1 && startHarvestFlag == 0) {
    statusUpdate = "Cultervating";
  }

  if (pbrAM == 0 && pbrSS == 0) {
    statusUpdate = "System Off";
  }

  if (startCycleFlag == 2 && startHarvestFlag == 1) {
    flashMillis = millis();
    if (flashMillis - previousFlashMillis >= flashInterval) {
      previousFlashMillis = flashMillis;
      if (flashState == 0) {
        statusUpdate = "Harvesting";
        flashState = 1;
      } else {
        statusUpdate = " ";
        flashState = 0;
      }
    }
  }

  alarmUpdate = "All G";

}