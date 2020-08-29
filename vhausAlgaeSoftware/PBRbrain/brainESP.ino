#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

unsigned long updateCurrentMillis = 0;
unsigned long lastUpdateDelay = 0;
unsigned long updateDelay = 5000;

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

int updateCycle, cycleCheck, pbrCycleWeeks, pbrCycleDays, pbrCycleHours, pbrCycleMinuets;

float luxPV, phPV, doPV, tempPV, TurbPV, co2InPV, co2OutPV, presPV;
float luxSV, phSV, doSV, tempSV, TurbSV, co2InSV, co2OutSV, presSV;
float luxSVLast, phSVLast, doSVLast, tempSVLast, TurbSVLast, co2InSVLast, co2OutSVLast, presSVLast;

int press_valve_sv, dump1_valve_sv, dump2_valve_sv;
int press_valve_svLast, dump1_valve_svLast, dump2_valve_svLast;

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
  if (topicStr == "pbr/pbrSS/switch") {
    if (payloadStr == "ON") client.publish("pbr/pbrSS/status", "ON"), pbrSS = 1;
    else if (payloadStr == "OFF") client.publish("pbr/pbrSS/status", "OFF"), pbrSS = 0;
  }

  if (topicStr == "pbr/lightAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/lightAM/status", "ON"), lightAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/lightAM/status", "OFF"), lightAM = 0;
  }
  if (topicStr == "pbr/lp1/switch") {
    if (payloadStr == "ON") client.publish("pbr/lp1/status", "ON"), lp1 = 1;
    else if (payloadStr == "OFF") client.publish("pbr/lp1/status", "OFF"), lp1 = 0;
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
    if (payloadStr == "ON") client.publish("pbr/lp2/status", "ON"), lp2 = 1;
    else if (payloadStr == "OFF") client.publish("pbr/lp2/status", "OFF"), lp2 = 0;
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

  if (topicStr == "pbr/chillAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/chillAM/status", "ON"), chillAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/chillAM/status", "OFF"), chillAM = 0;
  }
  if (topicStr == "pbr/chillSS/switch") {
    if (payloadStr == "ON") client.publish("pbr/chillSS/status", "ON"), chillSS = 1;
    else if (payloadStr == "OFF") client.publish("pbr/chillSS/status", "OFF"), chillSS = 0;
  }

  if (topicStr == "pbr/airAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/airAM/status", "ON"), airAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/airAM/status", "OFF"), airAM = 0;
  }
  if (topicStr == "pbr/airSS/switch") {
    if (payloadStr == "ON") client.publish("pbr/airSS/status", "ON"), airSS = 1;
    else if (payloadStr == "OFF") client.publish("pbr/airSS/status", "OFF"), airSS = 0;
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
    if (payloadStr == "ON") client.publish("pbr/topUp/status", "ON"), topUp = 1;
    else if (payloadStr == "OFF") client.publish("pbr/topUp/status", "OFF"), topUp = 0;
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

  if (topicStr == "press_valve_sv") press_valve_sv = payloadStr.toInt();
  if (topicStr == "dump1_valve_sv") dump1_valve_sv = payloadStr.toInt();
  if (topicStr == "dump2_valve_sv") dump2_valve_sv = payloadStr.toInt();
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

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  setup_wifi(); // Connect to network

  updateCycle = pbrCycleWeeks + pbrCycleDays + pbrCycleHours + pbrCycleMinuets;

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


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateCurrentMillis = millis(); //send update every x seconds
  if (updateCurrentMillis - lastUpdateDelay >= updateDelay) {
    lastUpdateDelay = updateCurrentMillis;
    //packetUpDate();  //dump data
  }
  updateTimers();
  updateInputNumbers();
  updateServos();

}

void updateTimers() {
  updateCycle = pbrCycleWeeks + pbrCycleDays + pbrCycleHours + pbrCycleMinuets;
  if (updateCycle != cycleCheck) {
    //convert to minuets for display
    updateCycle = pbrCycleWeeks * 10080;
    updateCycle = updateCycle + pbrCycleDays * 1440;
    updateCycle = updateCycle + pbrCycleHours * 60;
    updateCycle = updateCycle + pbrCycleMinuets;
    String str = String(updateCycle);
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    client.publish("pbrCycleTime", char_array, str_len);
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

void packetUpDate() {
  const size_t capacity = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument doc1(capacity);
  DynamicJsonDocument doc2(capacity);
  doc1["holdingPV"] = luxPV;
  char buffer[256];
  size_t n = serializeJson(doc1, buffer);
  client.publish("brainOutPV1", buffer, n);
  doc2["holdingSV"] = luxSV;
  n = serializeJson(doc2, buffer);
  client.publish("brainOutSV1", buffer, n);
}