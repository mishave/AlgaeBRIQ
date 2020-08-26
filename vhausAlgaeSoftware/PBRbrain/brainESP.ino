#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

unsigned long updateCurrentMillis;
unsigned long lastUpdateDelay;
unsigned long updateDelay = 1000;

// MQTT Network
const char* mqtt_server = "192.168.0.200";
//Node ID
const char *ID = "brainESP32";  // Name of our device, must be unique
//Topics
const char *TOPIC = "room/light";  // Topic to subcribe to
const char *STATE_TOPIC = "room/light/state";  // Topic to publish the light state to
int officeLight1Status;

// Home Assistant Credentials
const char *HA_USER = "vhausTech";
const char *HA_PASS = "vhaus";

int pbrAM, pbrSS, lightAM,
    lp1, lp1_1, lp1_2, lp1_3, lp1_4,
    lp2, lp2_1, lp2_2, lp2_3, lp2_4;

int pbrAML, pbrSSL, lightAML,
    lp1L, lp1_1L, lp1_2L, lp1_3L, lp1_4L,
    lp2L, lp2_1L, lp2_2L, lp2_3L, lp2_4L;

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
    if (payloadStr == "ON") pbrAM = 1;
    else if (payloadStr == "OFF") pbrAM = 0;
    pbrAML=1;
  }
  if (topicStr == "pbr/pbrSS/switch") {
    if (payloadStr == "ON") pbrSS = 1;
    else pbrSS = 0;
    pbrSSL=1;
  }
  if (topicStr == "pbr/lightAM/switch") {
    if (payloadStr == "ON") lightAM = 1;
    else lightAM = 0;
    lightAML=1;
  }
  if (topicStr == "pbr/lp1/switch") {
    if (payloadStr == "ON") lp1 = 1;
    else lp1 = 0;
    lp1L=1;
  }
  if (topicStr == "pbr/lp1_1/switch") {
    if (payloadStr == "ON") lp1_1 = 1;
    else lp1_1 = 0;
    lp1_1L=1;
  }
  /*lp1_2, lp1_3, lp1_4,
      lp2, lp2_1, lp2_2, lp2_3, lp2_4;
    }*/
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


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(100);
  switchUpdate();
  updateCurrentMillis = millis(); //send update every x seconds
  if (updateCurrentMillis - lastUpdateDelay >= updateDelay) {
    lastUpdateDelay = updateCurrentMillis;
    //switchOut();  //dump data
  }
}
void switchUpdate() {
  if (pbrAML == 1) {
    if (pbrAM == 1) {
      client.publish("PBR/pbrAM/status", "ON");
      Serial.println("its on");
    }
    else {
      client.publish("PBR/pbrAM/status", "Off");
      Serial.println("its off");
    }
    pbrAML = 0;
  }
  if (pbrSS != 3) {
    if (pbrSS == 1) client.publish("PBR/pbrSS/status", "ON");
    else client.publish("PBR/pbrSS/status", "Off");
    pbrSS = 3;
  }
  if (lightAML == 1) {
    if (lightAM == 1) client.publish("PBR/lightAM/status", "ON");
    else client.publish("PBR/lightAM/status", "Off");
    lightAM = 0;
  }
  if (lp1L == 1) {
    if (lp1 == 1) client.publish("PBR/lp1/status", "ON");
    else client.publish("PBR/lp1/status", "Off");
    lp1L = 0;
  }
  if (lp1_1L == 1) {
    if (lp1_1 == 1) client.publish("PBR/lp1_1/status", "ON");
    else client.publish("PBR/lp1_1/status", "Off");
    lp1_1L = 0;
  }
}
void switchOut() {
  const size_t capacity = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument doc1(capacity);
  DynamicJsonDocument doc2(capacity);
  doc1["pbrAM"] = 1;
  doc1["pbrSS"] = 1;
  doc1["lightAM"] = 0;
  doc1["lp_1"] = 0;
  doc1["lp1_1"] = 0;
  doc1["lp1_2"] = 0;
  doc1["lp1_3"] = 1;
  doc1["lp1_4"] = 0;
  doc1["lp_2"] = 0;
  doc1["lp2_1"] = 1;
  doc1["lp2_2"] = 0;
  doc1["lp2_3"] = 0;
  doc1["lp2_4"] = 0;
  char buffer[256];
  size_t n = serializeJson(doc1, buffer);
  client.publish("brainOut1", buffer, n);
  doc2["tempAM"] = 0;
  doc2["tempSS"] = 1;
  doc2["airAM"] = 0;
  doc2["airSS"] = 0;
  doc2["doseAM"] = 0;
  doc2["doseSS"] = 1;
  doc2["dosephU"] = 0;
  doc2["dosepHD"] = 0;
  doc2["doseNut"] = 0;
  doc2["doseSam"] = 0;
  doc2["doseFill"] = 0;
  doc2["harvsetAM"] = 0;
  doc2["harvestSS"] = 0;
  n = serializeJson(doc2, buffer);
  client.publish("brainOut2", buffer, n);
}